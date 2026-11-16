// Processor: parameters from map/params.csv, MIDI in and out, state. Header only so the editor and the offscreen snapshot tool can share it.
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <memory>
#include <map>
#include <array>
#include "BinaryData.h"
#include "MidiCore.h"
#include "PageData.h"

namespace ctlns
{
constexpr int kMaxParams = 400;     // keeps the host's parameter list reasonable
constexpr int kPerBlock = 16;       // at most this many parameters are sent per audio block, so a device is never flooded

inline ctl::ParseResult loadMap()
{
    return ctl::parseMap(std::string(BinaryData::params_csv, size_t(BinaryData::params_csvSize)));
}
}
using namespace ctlns;

class CtlProcessor : public juce::AudioProcessor, private juce::AudioProcessorParameter::Listener, private juce::Timer
{
public:
    CtlProcessor() : AudioProcessor(BusesProperties()), mapRes(loadMap()), decoder(mapRes.params)
    {
        auto ch = std::make_unique<juce::AudioParameterInt>(juce::ParameterID{ "midi_channel", 1 }, "MIDI channel", 1, 16, 1);
        channelParam = ch.get(); addParameter(ch.release());
        for (auto& p : mapRes.params)
        {
            if (p.kind == ctl::Kind::Pair || int(slots.size()) >= kMaxParams) continue;
            auto ip = std::make_unique<juce::AudioParameterInt>(juce::ParameterID{ p.id, 1 }, p.name, p.minV, p.maxV, p.defV);
            Slot s; s.param = ip.get(); s.map = int(&p - mapRes.params.data()); slots.push_back(s);
            addParameter(ip.release());
        }
        for (int i = 0; i < pd::kNumKeys + 8; ++i) { uiVal[i] = 0; uiSet[i] = false; }
        pending = std::make_unique<std::atomic<int>[]>(slots.size()); lastRecv = std::make_unique<std::atomic<int>[]>(slots.size()); incoming = std::make_unique<std::atomic<int>[]>(slots.size());
        for (size_t i = 0; i < slots.size(); ++i) { pending[i] = -1; lastRecv[i] = -1; incoming[i] = -1; slots[i].param->addListener(this); }
        startTimerHz(30);
    }
    ~CtlProcessor() override { stopTimer(); for (auto& s : slots) s.param->removeListener(this); }

    const ctl::ParseResult mapRes;
    struct Slot { juce::AudioParameterInt* param = nullptr; int map = 0; };
    std::vector<Slot> slots;
    juce::AudioParameterInt* channelParam = nullptr;

    void sendAll() { for (size_t i = 0; i < slots.size(); ++i) pending[i] = slots[i].param->get(); }

    // index into slots for a parameter id such as "cc_34", or -1
    int findSlot(const juce::String& id) const
    {
        for (size_t i = 0; i < slots.size(); ++i) if (slots[i].param->paramID == id) return int(i);
        return -1;
    }

    // Editor state that is not a device parameter (button states, values of controls the map has no CC for). Saved with the project, never sent.
    std::atomic<int> uiVal[pd::kNumKeys + 8];
    std::atomic<bool> uiSet[pd::kNumKeys + 8];
    int getUi(int key, int def) const { return uiSet[key].load() ? uiVal[key].load() : def; }
    void setUi(int key, int v) { uiVal[key] = v; uiSet[key] = true; }

    // Raw MIDI from the editor (on-screen keys, program change). Lock free, read in processBlock.
    void queueMidi(int status, int d1, int d2)
    {
        int s1, n1, s2, n2; fifo.prepareToWrite(1, s1, n1, s2, n2);
        if (n1 > 0) { ring[size_t(s1)] = uint32_t(status) | (uint32_t(d1) << 8) | (uint32_t(d2) << 16); fifo.finishedWrite(1); }
    }
    void queueNote(int note, bool on) { queueMidi((on ? 0x90 : 0x80) | ((channelParam->get() - 1) & 15), juce::jlimit(0, 127, note), on ? 100 : 0); }
    // documented: send a bank change (0 to 4), then a program change (0 to 99)
    void queueProgram(int bank, int program)
    {
        for (const auto& r : ctl::programChange(channelParam->get(), bank, program)) queueMidi(r.b[0], r.b[1], r.len > 2 ? r.b[2] : 0);
    }

    void prepareToPlay(double sr, int) override { decoder.reset(); sampleRate = sr > 0 ? sr : 48000.0; guard.reset(); guard.setWindow(int64_t(sampleRate * 0.030)); }
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override { return true; }

    void processBlock(juce::AudioBuffer<float>& audio, juce::MidiBuffer& midi) override
    {
        juce::ScopedNoDenormals nd; audio.clear();
        // 1. incoming. Everything is passed on (the plugin is a pass-through by default); mapped CCs also move the knobs.
        //    A message identical to one the plugin just sent is the synth echoing it back (USB soft thru): it is dropped (loop guard).
        juce::MidiBuffer passthru;
        for (const auto meta : midi)
        {
            const auto msg = meta.getMessage(); const auto* raw = msg.getRawData(); const int len = msg.getRawDataSize();
            if (len >= 1 && len <= 3 && guard.isEcho(raw, len, clock + meta.samplePosition)) continue;
            if (msg.isController() && msg.getChannel() == juce::jlimit(1, 16, channelParam->get()))   // other channels do not move the knobs
                if (auto e = decoder.feedCC(msg.getControllerNumber(), msg.getControllerValue()))
                    for (size_t i = 0; i < slots.size(); ++i) if (slots[i].map == e->paramIndex) { incoming[i] = e->value; break; }
            passthru.addEvent(msg, meta.samplePosition);
        }
        midi.swapWith(passthru);
        // 2. outgoing: raw MIDI queued by the editor (keys, program change), then changed parameters, a few per block
        int pos = 0; const int lastPos = juce::jmax(0, audio.getNumSamples() - 1);
        { int s1, n1, s2, n2; fifo.prepareToRead(fifo.getNumReady(), s1, n1, s2, n2);
          auto emit = [&](int s, int n) { for (int k = 0; k < n; ++k) { const uint32_t w = ring[size_t(s + k)]; const int st = int(w & 255), d1 = int((w >> 8) & 127), d2 = int((w >> 16) & 127);
              const juce::uint8 b[3] = { juce::uint8(st), juce::uint8(d1), juce::uint8(d2) }; midi.addEvent(b, (st & 0xF0) == 0xC0 ? 2 : 3, juce::jmin(pos++, lastPos)); } };
          emit(s1, n1); emit(s2, n2); fifo.finishedRead(n1 + n2); }
        const int ch = channelParam->get(); const int n = int(slots.size()); int sent = 0;
        for (int k = 0; k < n && sent < kPerBlock; ++k)
        {
            const int i = (rotate + k) % n; const int v = pending[size_t(i)].exchange(-1); if (v < 0) continue;
            for (const auto& m : ctl::encode(mapRes.params[size_t(slots[size_t(i)].map)], ch, v))
            { const juce::uint8 bytes[3] = { m.status, m.d1, m.d2 }; midi.addEvent(bytes, 3, juce::jmin(pos++, lastPos)); }
            ++sent; rotate = (i + 1) % n;
        }
        for (const auto meta : midi) { const int len = meta.numBytes; if (len >= 1 && len <= 3) guard.sent(meta.data, len, clock + meta.samplePosition); }
        clock += audio.getNumSamples();
    }

    juce::AudioProcessorEditor* createEditor() override;   // defined in Plugin.cpp
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Cobalt8M Controller"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return true; }
    double getTailLengthSeconds() const override { return 0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& dest) override
    {
        juce::XmlElement x("STATE"); x.setAttribute("v", 1); x.setAttribute("midi_channel", channelParam->get());
        for (auto& s : slots) x.setAttribute(s.param->paramID, s.param->get());
        for (int i = 0; i < pd::kNumKeys + 8; ++i) if (uiSet[i].load()) x.setAttribute("u" + juce::String(i), uiVal[i].load());
        copyXmlToBinary(x, dest);
    }
    void setStateInformation(const void* data, int size) override
    {
        if (auto x = getXmlFromBinary(data, size))
        {
            if (!x->hasTagName("STATE")) return;
            const int ch = x->getIntAttribute("midi_channel", 1); *channelParam = juce::jlimit(1, 16, ch);
            for (auto& s : slots) if (x->hasAttribute(s.param->paramID)) setFromState(s, x->getIntAttribute(s.param->paramID));
            for (int i = 0; i < pd::kNumKeys + 8; ++i) if (x->hasAttribute("u" + juce::String(i))) setUi(i, x->getIntAttribute("u" + juce::String(i)));
        }
    }

private:
    void setFromState(Slot& s, int v)
    {
        const size_t i = size_t(&s - slots.data()); const auto r = s.param->getRange();
        v = juce::jlimit(r.getStart(), r.getEnd(), v); lastRecv[i] = v; *s.param = v;   // restoring a project does not send MIDI
    }
    void parameterValueChanged(int, float) override {}
    void parameterGestureChanged(int, bool) override {}
    void timerCallback() override
    {
        if (lastSeen.size() != slots.size()) { lastSeen.resize(slots.size()); for (size_t i = 0; i < slots.size(); ++i) lastSeen[i] = slots[i].param->get(); }
        // device to host: a parameter set from incoming MIDI is never sent back out
        for (size_t i = 0; i < slots.size(); ++i)
        {
            const int v = incoming[i].exchange(-1); if (v < 0 || slots[i].param->get() == v) continue;
            lastRecv[i] = v; *slots[i].param = v;
        }
        // host or UI to device
        for (size_t i = 0; i < slots.size(); ++i)
        {
            const int v = slots[i].param->get(); if (v == lastSeen[i]) continue;
            lastSeen[i] = v;
            if (lastRecv[i].load() == v) lastRecv[i] = -1; else pending[i] = v;
        }
    }

    ctl::Decoder decoder; ctl::LoopGuard guard; double sampleRate = 48000.0; int64_t clock = 0;
    juce::AbstractFifo fifo { 256 }; std::array<uint32_t, 256> ring {};
    std::unique_ptr<std::atomic<int>[]> pending, lastRecv, incoming;
    std::vector<int> lastSeen; int rotate = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtlProcessor)
};

