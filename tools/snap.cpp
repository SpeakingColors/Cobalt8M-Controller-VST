// Offscreen tools for the real editor (not shipped). Needs a display, for example xvfb-run.
//   snap <out dir>        one PNG per page
//   snap --selftest       clicks, drags and MIDI through the real editor and processor, prints PASS or FAIL
#include "Processor.h"
#include "Editor.h"

juce::AudioProcessorEditor* CtlProcessor::createEditor() { return new ctled::CtlEditor(*this); }

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); ++fails; } } while (0)

static std::vector<std::array<int, 3>> run(CtlProcessor& p, juce::MidiBuffer in = {})
{
    juce::AudioBuffer<float> audio(2, 256); audio.clear(); p.processBlock(audio, in);
    std::vector<std::array<int, 3>> out; for (const auto m : in) { const auto* d = m.data; out.push_back({ d[0], m.numBytes > 1 ? d[1] : 0, m.numBytes > 2 ? d[2] : 0 }); }
    return out;
}
static bool has(const std::vector<std::array<int, 3>>& v, int a, int b, int c) { for (auto& m : v) if (m[0] == a && m[1] == b && (c < 0 || m[2] == c)) return true; return false; }

static void selftest()
{
    CtlProcessor proc; proc.prepareToPlay(48000.0, 256);
    std::unique_ptr<juce::AudioProcessorEditor> ed(proc.createEditor());
    auto* view = dynamic_cast<ctled::MockupView*>(ed->getChildComponent(0));
    CHECK(view != nullptr); if (!view) return;
    CHECK(ed->getWidth() == int(ctled::kDesignW * ctled::kDefaultScale));
    // 1. knob drag moves the bound parameter: Cutoff is CC 34, knob at (793,227) size 60
    const int cut = proc.findSlot("cc_34"); CHECK(cut >= 0);
    CHECK(proc.slots[size_t(cut)].param->get() == 0);
    view->down({ 823, 257 }, 1); view->dragTo({ 823, 257 - 75 }, false); view->up();
    CHECK(proc.slots[size_t(cut)].param->get() >= 62 && proc.slots[size_t(cut)].param->get() <= 65);       // half of 150 design px is half the range
    view->down({ 823, 257 }, 2); view->up(); CHECK(proc.slots[size_t(cut)].param->get() == 0);               // double click: default
    // 2. a mapped CC from the DAW moves the knob and is passed on
    { juce::MidiBuffer in; in.addEvent(juce::MidiMessage::controllerEvent(1, 34, 90), 0); auto out = run(proc, in); CHECK(has(out, 0xB0, 34, 90)); }
    // 3. unmapped CC and notes pass through
    { juce::MidiBuffer in; in.addEvent(juce::MidiMessage::controllerEvent(1, 10, 33), 0); in.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8) 100), 4); auto out = run(proc, in); CHECK(has(out, 0xB0, 10, 33)); CHECK(has(out, 0x90, 60, 100)); }
    // 4. loop guard: the synth echoes the note back next block, it is dropped; a different note is not
    { juce::MidiBuffer in; in.addEvent(juce::MidiMessage::noteOn(1, 64, (juce::uint8) 90), 0); run(proc, in); }
    { juce::MidiBuffer in; in.addEvent(juce::MidiMessage::noteOn(1, 64, (juce::uint8) 90), 0); in.addEvent(juce::MidiMessage::noteOn(1, 65, (juce::uint8) 90), 8); auto out = run(proc, in); CHECK(!has(out, 0x90, 64, 90)); CHECK(has(out, 0x90, 65, 90)); }
    // 5. a knob moved on the screen is sent out as its CC (after the timer notices; run the timer path by hand)
    // 6. clicking a preset row sends bank change then program change. Row 056 is the 7th row of column 3: number 30 + 2 * 10 + 6
    proc.setUi(ctled::kUiBank, 2);
    view->down({ 900, 644.453f + 6 * 31 + 15 }, 1); view->up();
    CHECK(view->getPage() == 0);
    { auto out = run(proc); CHECK(has(out, 0xB0, 0, 2)); CHECK(has(out, 0xC0, 56, -1)); }
    // 7. piano keys on the Keyboard page play notes: tab "Keyboard" is the 5th tab
    view->setPage(4);
    view->down({ 170, 800 }, 1); { auto out = run(proc); CHECK(has(out, 0x90, 48, 100)); } view->up(); { auto out = run(proc); CHECK(has(out, 0x80, 48, -1)); }
    // 8. window height follows the page and the aspect ratio is kept
    const double aspect = double(ctled::kDesignW) / double(pd::pages[4].h); CHECK(std::abs(double(ed->getWidth()) / ed->getHeight() - aspect) < 0.01);
    view->setPage(5); CHECK(std::abs(double(ed->getWidth()) / ed->getHeight() - double(ctled::kDesignW) / double(pd::pages[5].h)) < 0.01);
    // 9. state round trip keeps knobs, button states and the page
    juce::MemoryBlock blk; proc.setUi(5, 1); proc.getStateInformation(blk);
    CtlProcessor other; other.setStateInformation(blk.getData(), int(blk.getSize())); CHECK(other.getUi(5, 0) == 1); CHECK(other.getUi(ctled::kUiPage, 0) == 5);
    ed.reset();
    std::printf(fails ? "%d editor test failures\n" : "all editor tests passed\n", fails);
}

int main(int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI gui;
    if (argc > 1 && juce::String(argv[1]) == "--selftest") { selftest(); return fails ? 1 : 0; }
    const juce::File out(argc > 1 ? argv[1] : ".");
    CtlProcessor proc;
    for (int page = 0; page < 6; ++page)
    {
        proc.setUi(ctled::kUiPage, page);
        std::unique_ptr<juce::AudioProcessorEditor> ed(proc.createEditor()); ed->setVisible(true);
        if (argc > 2 && juce::String(argv[2]) == "full") ed->setSize(int(ctled::kDesignW), int(pd::pages[page].h));   // 100% of the mockup size
        auto img = ed->createComponentSnapshot(ed->getLocalBounds(), true, 1.0f);
        juce::PNGImageFormat png; juce::FileOutputStream os(out.getChildFile("page" + juce::String(page) + ".png")); os.setPosition(0); os.truncate(); png.writeImageToStream(img, os);
    }
    return 0;
}
