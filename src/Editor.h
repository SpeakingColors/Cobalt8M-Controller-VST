// Editor: the six mockup pages drawn natively. The layout comes from src/PageData.h (generated from docs/mockups by tools/port_mockups.py),
// so the window is the mockup, not a stand-in. Knobs and buttons with a documented CC are bound to the processor parameters;
// everything else keeps its state in the project (and is never sent to the device).
#pragma once
#include <functional>
#include <cmath>
#include "Processor.h"
#include "catui/CatLookAndFeel.h"
#include "catui/RenderGate.h"

namespace ctled
{
constexpr float kDesignW = 1668.0f;
constexpr float kDefaultScale = 0.72f;     // 100% = this size; the mockup is drawn at full size when scaled to 1.0
constexpr int kUiPage = pd::kNumKeys, kUiBank = pd::kNumKeys + 1, kUiPreset = pd::kNumKeys + 2, kUiBubbles = pd::kNumKeys + 3;

inline juce::Colour col(uint32_t c) { return juce::Colour(juce::uint8((c >> 16) & 255), juce::uint8((c >> 8) & 255), juce::uint8(c & 255), juce::uint8((c >> 24) & 255)); }

class MockupView : public juce::Component, public juce::TooltipClient
{
public:
    MockupView(CtlProcessor& p, std::function<void(int)> pageChanged, std::function<void()> resetSize, std::function<void(juce::Point<int>)> showInfoMenu)
        : proc(p), onPage(std::move(pageChanged)), onReset(std::move(resetSize)), onInfo(std::move(showInfoMenu)), gate(*this)
    {
        page = juce::jlimit(0, 5, proc.getUi(kUiPage, 0));
        gate.onFrame = [this] { if (changedSinceLastFrame()) repaint(); };
        gate.start(20);
        buildPresetList();
        setOpaque(true);
    }

    int getPage() const { return page; }
    float designHeight() const { return pd::pages[page].h; }
    void setPage(int p) { page = juce::jlimit(0, 5, p); proc.setUi(kUiPage, page); onPage(page); repaint(); }

    //==============================================================================
    void paint(juce::Graphics& g) override
    {
        const auto& pg = pd::pages[page];
        const float s = getWidth() / kDesignW;
        g.fillAll(juce::Colour(0xff0b0b0c));
        g.addTransform(juce::AffineTransform::scale(s));
        // wood cheeks, the mockup's 34 px each side
        catui::drawWood(g, juce::Rectangle<int>(0, 0, 34, int(pg.h)));
        catui::drawWood(g, juce::Rectangle<int>(int(kDesignW) - 34, 0, 34, int(pg.h)));
        juce::Graphics::ScopedSaveState clip(g);
        g.reduceClipRegion(34, 0, int(kDesignW) - 68, int(pg.h));
        for (int n = 0; n < pg.nItems; ++n)
        {
            const auto it = pg.items[n];
            switch (it.k)
            {
                case 'b': drawBox(g, pg.boxes[it.i]); break;
                case 't': drawText(g, pg.texts[it.i]); break;
                case 'k': drawKnob(g, pg.knobs[it.i]); break;
                case 'n': drawButton(g, pg.btns[it.i]); break;
                case 's': drawSvg(g, pg.svgs[it.i]); break;
                default: break;
            }
        }
    }

    //==============================================================================
    void mouseDown(const juce::MouseEvent& e) override { down(toDesign(e.position), e.getNumberOfClicks()); }
    void mouseDrag(const juce::MouseEvent& e) override { dragTo(toDesign(e.position), e.mods.isShiftDown()); }
    void mouseUp(const juce::MouseEvent&) override { up(); }
    // The same actions in design coordinates (the mockup's pixels); the mouse handlers and the self test both use them.
    void down(juce::Point<float> p, int clicks)
    {
        const auto& pg = pd::pages[page];
        dragKnob = -1; dragSlider = -1; heldKey = -1; downButton = -1;
        if (p.x < 34 || p.x > kDesignW - 34) { if (clicks > 1) onReset(); return; }
        for (int i = 0; i < pg.nLinks; ++i)
            if (inside(pg.links[i].x, pg.links[i].y, pg.links[i].w, pg.links[i].h, p)) { setPage(pg.links[i].page); return; }
        // preset rows (Preset Manager page)
        if (page == 0) { const int row = presetAt(p); if (row >= 0) { choosePreset(row); return; } }
        for (int i = nButtons() - 1; i >= 0; --i)
        {
            const auto& b = pg.btns[i];
            if (b.w <= 0 || !inside(b.x, b.y, b.w, b.h, p)) continue;
            if (b.act == pd::ActSlider) { dragSlider = i; slideTo(i, p.x); return; }
            if (b.act == pd::ActKey) { heldKey = i; keyDown = true; proc.queueNote(b.offv, true); repaint(); return; }
            pressButton(i); return;
        }
        for (int i = nKnobs() - 1; i >= 0; --i)
        {
            const auto& k = pg.knobs[i];
            if (k.size <= 0 || !inside(k.x - 4, k.y - 4, k.size + 8, k.size + 8, p)) continue;
            dragKnob = i; dragStart = p.y; dragStartVal = knobValue(k);
            if (auto* pa = paramFor(k.param)) pa->beginChangeGesture();
            if (clicks > 1) setKnob(k, defaultFor(k));
            return;
        }
    }
    void dragTo(juce::Point<float> p, bool fine)
    {
        const auto& pg = pd::pages[page];
        if (dragKnob >= 0)
        {
            const float per = fine ? 600.0f : 150.0f;   // design pixels for the full range
            const float dy = dragStart - p.y;
            setKnob(pg.knobs[dragKnob], int(std::round(dragStartVal + dy * 127.0f / per)));
        }
        else if (dragSlider >= 0) slideTo(dragSlider, p.x);
    }
    void up()
    {
        const auto& pg = pd::pages[page];
        if (dragKnob >= 0) { if (auto* pa = paramFor(pg.knobs[dragKnob].param)) pa->endChangeGesture(); dragKnob = -1; }
        if (heldKey >= 0) { proc.queueNote(pg.btns[heldKey].offv, false); heldKey = -1; repaint(); }
        if (momentary >= 0) { releaseMomentary(); }
        dragSlider = -1;
    }
    void mouseMove(const juce::MouseEvent& e) override { hover = toDesign(e.position); }
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& w) override
    {
        const auto p = toDesign(e.position); const auto& pg = pd::pages[page];
        for (int i = 0; i < nKnobs(); ++i)
        {
            const auto& k = pg.knobs[i];
            if (k.size > 0 && inside(k.x, k.y, k.size, k.size, p)) { setKnob(k, knobValue(k) + (w.deltaY > 0 ? 1 : -1) * (e.mods.isShiftDown() ? 1 : 3)); return; }
        }
    }

    juce::String getTooltip() override
    {
        if (proc.getUi(kUiBubbles, 1) == 0) return {};
        const auto& pg = pd::pages[page];
        for (int i = 0; i < nKnobs(); ++i)
        {
            const auto& k = pg.knobs[i];
            if (k.size > 0 && inside(k.x, k.y, k.size, k.size, hover)) return describe(juce::String::fromUTF8(k.label), k.param);
        }
        for (int i = 0; i < nButtons(); ++i)
        {
            const auto& b = pg.btns[i];
            if (b.w > 0 && inside(b.x, b.y, b.w, b.h, hover) && (b.param[0] || b.act == pd::ActSlider)) return describe(juce::String::fromUTF8(b.text), b.param);
            if (b.w > 0 && inside(b.x, b.y, b.w, b.h, hover) && b.act == pd::ActKey) return "Plays note " + juce::MidiMessage::getMidiNoteName(b.offv, true, true, 4) + " on the device (MIDI channel " + juce::String(proc.channelParam->get()) + ").";
        }
        return {};
    }

private:
    //==============================================================================
    CtlProcessor& proc; std::function<void(int)> onPage; std::function<void()> onReset; std::function<void(juce::Point<int>)> onInfo;
    catui::RenderGate gate;
    int page = 0, dragKnob = -1, dragSlider = -1, heldKey = -1, downButton = -1, momentary = -1; bool keyDown = false;
    float dragStart = 0; int dragStartVal = 0; juce::Point<float> hover;
    std::vector<int> lastValues;
    struct PresetRow { int num; juce::String name; };
    std::vector<PresetRow> presets;     // from the mockup list (names are the factory ones shown there)

    int nButtons() const { return btnCount(page); }
    int nKnobs() const { return knobCount(page); }
    static int knobCount(int p) { static int c[6] = { -1, -1, -1, -1, -1, -1 }; if (c[p] < 0) { c[p] = 0; for (int i = 0; i < pd::pages[p].nItems; ++i) c[p] += pd::pages[p].items[i].k == 'k'; } return c[p]; }
    static int btnCount(int p) { static int c[6] = { -1, -1, -1, -1, -1, -1 }; if (c[p] < 0) { c[p] = 0; for (int i = 0; i < pd::pages[p].nItems; ++i) c[p] += pd::pages[p].items[i].k == 'n'; } return c[p]; }
    juce::Point<float> toDesign(juce::Point<float> p) const { const float s = getWidth() / kDesignW; return { p.x / s, p.y / s }; }
    static bool inside(float x, float y, float w, float h, juce::Point<float> p) { return p.x >= x && p.x <= x + w && p.y >= y && p.y <= y + h; }

    juce::AudioParameterInt* paramFor(const char* id) const
    {
        if (!id || !id[0]) return nullptr;
        const int i = proc.findSlot(id); return i < 0 ? nullptr : proc.slots[size_t(i)].param;
    }
    int knobValue(const pd::Knob& k) const { if (auto* pa = paramFor(k.param)) return pa->get(); return proc.getUi(k.key, k.init); }
    int defaultFor(const pd::Knob& k) const
    {
        const int si = k.param[0] ? proc.findSlot(k.param) : -1;
        return si >= 0 ? proc.mapRes.params[size_t(proc.slots[size_t(si)].map)].defV : k.init;
    }
    void setKnob(const pd::Knob& k, int v)
    {
        if (auto* pa = paramFor(k.param)) { const auto r = pa->getRange(); const int c = juce::jlimit(r.getStart(), r.getEnd(), v); pa->setValueNotifyingHost(pa->convertTo0to1(float(c))); }
        else proc.setUi(k.key, juce::jlimit(0, 127, v));
        repaint();
    }
    bool changedSinceLastFrame()
    {
        if (lastValues.size() != proc.slots.size()) { lastValues.assign(proc.slots.size(), -999); }
        bool ch = false;
        for (size_t i = 0; i < proc.slots.size(); ++i) { const int v = proc.slots[i].param->get(); if (v != lastValues[i]) { lastValues[i] = v; ch = true; } }
        return ch;
    }

    //==============================================================================
    void pressButton(int i)
    {
        const auto& b = pd::pages[page].btns[i];
        switch (b.act)
        {
            case pd::ActReset100: onReset(); return;
            case pd::ActInfo: onInfo(getScreenPosition()); return;
            case pd::ActSettings: setPage(5); return;
            case pd::ActPcPrev: stepPreset(-1); return;
            case pd::ActPcNext: stepPreset(1); return;
            case pd::ActBank: { const int bank = juce::String::fromUTF8(b.text).getTrailingIntValue(); proc.setUi(kUiBank, bank); repaint(); return; }
            default: break;
        }
        // radio group: any button of the group is selected in the data and the group has more than one member
        bool radio = false; int members = 0;
        for (int j = 0; j < nButtons(); ++j) { const auto& o = pd::pages[page].btns[j]; if (o.group == b.group && o.act != pd::ActKey) { ++members; if (o.sel) radio = true; } }
        if (radio && members > 1)
        {
            for (int j = 0; j < nButtons(); ++j) { const auto& o = pd::pages[page].btns[j]; if (o.group == b.group && o.act != pd::ActKey) proc.setUi(o.key, j == i ? 1 : 0); }
            if (auto* pa = paramFor(b.param)) setParam(pa, b.onv);
            repaint(); return;
        }
        const int cur = proc.getUi(b.key, b.sel);
        if (b.kind == 3) { if (auto* pa = paramFor(b.param)) setParam(pa, b.onv); momentary = i; proc.setUi(b.key, 1); repaint(); return; }
        const int next = cur ? 0 : 1;
        if (b.kind == 1 || b.kind == 2) { proc.setUi(b.key, next); if (auto* pa = paramFor(b.param)) setParam(pa, next ? b.onv : b.offv); }
        else if (b.sel || b.bg != b.selbg) { /* plain pill in the mockup: toggle its look only when it has a selected look */ if (b.sel) proc.setUi(b.key, next); }
        repaint();
    }
    void releaseMomentary()
    {
        const auto& b = pd::pages[page].btns[momentary]; proc.setUi(b.key, 0); momentary = -1; repaint();
    }
    static void setParam(juce::AudioParameterInt* pa, int v) { pa->beginChangeGesture(); pa->setValueNotifyingHost(pa->convertTo0to1(float(v))); pa->endChangeGesture(); }
    static float sliderStart(const pd::Btn& b)
    {
        juce::GlyphArrangement ga; ga.addLineOfText(font(b.fs > 1 ? b.fs : 13.0f, false, false), juce::String::fromUTF8(b.text), 0.0f, 0.0f);
        return b.x + 10.0f + ga.getBoundingBox(0, -1, true).getWidth() + 10.0f;
    }
    void slideTo(int i, float x)
    {
        const auto& b = pd::pages[page].btns[i]; auto* pa = paramFor(b.param); if (!pa) return;
        const float x0 = sliderStart(b), x1 = b.x + b.w - 10.0f;
        const float t = juce::jlimit(0.0f, 1.0f, (x - x0) / (x1 - x0)); const auto r = pa->getRange();
        pa->setValueNotifyingHost(pa->convertTo0to1(float(r.getStart()) + t * float(r.getEnd() - r.getStart()))); repaint();
    }

    //==============================================================================
    // preset list (Preset Manager page): rows are the number text items of the mockup list
    void buildPresetList()
    {
        const auto& pg = pd::pages[0];
        for (int n = 0; n + 1 < pg.nItems; ++n)
        {
            if (pg.items[n].k != 't') continue; const auto& t = pg.texts[pg.items[n].i]; const juce::String s = juce::String::fromUTF8(t.s);
            if (t.y > 640 && t.y < 960 && s.length() == 3 && s.containsOnly("0123456789"))
                for (int m = n + 1; m < pg.nItems && m < n + 4; ++m)
                    if (pg.items[m].k == 't') { presets.push_back({ s.getIntValue(), juce::String::fromUTF8(pg.texts[pg.items[m].i].s) }); break; }
        }
    }
    static int rowIndexOf(float x, float y) { const int col = int(std::lround((x - 359.0f) / 251.0f)), row = int(std::lround((y - 644.453f) / 31.0f)); return (col < 0 || col > 4 || row < 0 || row > 9) ? -1 : col * 10 + row; }
    int presetAt(juce::Point<float> p) const { if (p.x < 359 || p.x > 1614 || p.y < 644) return -1; return rowIndexOf(std::floor((p.x - 359.0f) / 251.0f) * 251.0f + 359.0f, std::floor((p.y - 644.453f) / 31.0f) * 31.0f + 644.453f); }
    void choosePreset(int row) { const int num = 30 + row; proc.setUi(kUiPreset, num + 1); proc.queueProgram(proc.getUi(kUiBank, 0), num); repaint(); }
    void stepPreset(int d)
    {
        const int cur = proc.getUi(kUiPreset, 56 + 1) - 1; const int next = juce::jlimit(0, 99, cur + d);
        proc.setUi(kUiPreset, next + 1); proc.queueProgram(proc.getUi(kUiBank, 0), next); repaint();
    }
    juce::String presetName(int num) const { for (auto& p : presets) if (p.num == num) return p.name; return {}; }

    //==============================================================================
    static juce::Font font(float fs, bool bold, bool mono)
    {
        auto f = mono ? juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), fs * 0.95f, juce::Font::plain) : juce::FontOptions(fs * 0.97f, bold ? juce::Font::bold : juce::Font::plain);
        return juce::Font(f);
    }
    void drawBox(juce::Graphics& g, const pd::Box& b)
    {
        juce::Rectangle<float> r(b.x, b.y, b.w, b.h);
        if (page == 0 && b.y > 640 && b.w > 249 && b.w < 253 && b.h > 29 && b.h < 33)
        {   // preset row background: the selected row is the dark one
            const int row = rowIndexOf(b.x, b.y); const bool selected = row >= 0 && 30 + row == proc.getUi(kUiPreset, 56 + 1) - 1;
            g.setColour(selected ? juce::Colour(0xff0b0b0c) : juce::Colour(0xff24558f)); g.fillRect(r); return;
        }
        if (b.dir == 3) { g.setColour(col(b.c2)); g.drawRoundedRectangle(r.reduced(0.5f), b.rad, 1.0f); return; }
        if (b.dir == 0) g.setColour(col(b.c));
        else g.setGradientFill(juce::ColourGradient(col(b.c), b.dir == 1 ? r.getX() : 0, b.dir == 2 ? r.getY() : 0, col(b.c2), b.dir == 1 ? r.getRight() : 0, b.dir == 2 ? r.getBottom() : 0, false));
        if (b.rad > 0.5f) g.fillRoundedRectangle(r, b.rad); else g.fillRect(r);
    }
    void drawText(juce::Graphics& g, const pd::Text& t)
    {
        g.setColour(col(t.c)); g.setFont(font(t.fs, t.bold, t.mono));
        g.drawFittedText(juce::String::fromUTF8(t.s), juce::Rectangle<int>(int(t.x) - 2, int(t.y) - 1, int(t.w) + 6, int(t.h) + 2), juce::Justification::centred, 1, 0.6f);
    }
    void drawKnob(juce::Graphics& g, const pd::Knob& k)
    {
        if (k.size <= 0) return;
        const float cx = k.x + k.size / 2, cy = k.y + k.size / 2, r = k.size * 0.425f;
        const int v = knobValue(k); int lo = 0, hi = 127;
        if (auto* pa = paramFor(k.param)) { lo = pa->getRange().getStart(); hi = pa->getRange().getEnd(); }
        const float t = hi > lo ? juce::jlimit(0.0f, 1.0f, float(v - lo) / float(hi - lo)) : 0.0f;
        const float a0 = juce::degreesToRadians(-135.0f), a1 = a0 + juce::degreesToRadians(270.0f) * t;
        g.setColour(col(k.track)); g.drawEllipse(cx - r, cy - r, 2 * r, 2 * r, k.size * 0.05f);
        if (t > 0.002f)
        {
            juce::Path arc; arc.addCentredArc(cx, cy, r, r, 0, a0, a1, true);
            g.setColour(col(k.arc)); g.strokePath(arc, juce::PathStrokeType(k.size * 0.06f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            const float dx = cx + std::sin(a1) * (r - k.size * 0.1f), dy = cy - std::cos(a1) * (r - k.size * 0.1f);
            g.fillEllipse(dx - k.size * 0.04f, dy - k.size * 0.04f, k.size * 0.08f, k.size * 0.08f);
        }
        g.setColour(col(k.fg)); g.setFont(font(k.size * 0.26f, false, false));
        g.drawText(juce::String(v), juce::Rectangle<float>(cx - r, cy - r * 0.45f, 2 * r, r * 0.9f), juce::Justification::centred);
    }
    void drawButton(juce::Graphics& g, const pd::Btn& b)
    {
        if (b.w <= 0) return;
        juce::Rectangle<float> r(b.x, b.y, b.w, b.h);
        bool sel = proc.getUi(b.key, b.sel) != 0; juce::String text = juce::String::fromUTF8(b.text);
        if (b.act == pd::ActKey) sel = (heldKey >= 0 && &pd::pages[page].btns[heldKey] == &b);
        if (b.act == pd::ActBank) sel = juce::String::fromUTF8(b.text).getTrailingIntValue() == proc.getUi(kUiBank, 0);
        if (page != 5 && (b.act == pd::ActNone) && b.x < 120 && b.y > 70 && b.y < 90 && text.length() == 3 && text.containsOnly("0123456789")) text = juce::String(proc.getUi(kUiPreset, 57) - 1).paddedLeft('0', 3);
        if (b.x > 100 && b.x < 110 && b.y > 70 && b.y < 90) { const auto nm = presetName(proc.getUi(kUiPreset, 57) - 1); if (nm.isNotEmpty()) text = nm; }
        const juce::Colour fill = col(sel ? b.selbg : b.bg);
        if (fill.getAlpha() > 0) { g.setColour(fill); if (b.rad > 0.5f) g.fillRoundedRectangle(r, b.rad); else g.fillRect(r); }
        if (b.bdw > 0 && (b.bd >> 24) > 0 && b.act != pd::ActKey) { g.setColour(col(b.bd).withMultipliedAlpha(0.8f)); g.drawRoundedRectangle(r.reduced(0.5f), b.rad, b.bdw); }
        g.setColour(col(b.fg)); g.setFont(font(b.fs > 1 ? b.fs : 13.0f, false, false));
        if (b.act == pd::ActSlider)
        {
            g.drawText(text, r.translated(10, 0), juce::Justification::centredLeft);
            auto* pa = paramFor(b.param); const float t = pa ? float(pa->get()) / float(juce::jmax(1, pa->getRange().getEnd())) : 0.0f;
            const float x0 = sliderStart(b), x1 = b.x + b.w - 10.0f, y = b.y + b.h / 2;
            g.setColour(col(b.fg).withAlpha(0.35f)); g.fillRoundedRectangle(x0, y - 2, x1 - x0, 4, 2);
            g.setColour(col(b.fg)); g.fillRoundedRectangle(x0, y - 2, (x1 - x0) * t, 4, 2);
            return;
        }
        if (b.act == pd::ActKey) { g.setColour(juce::Colour(0xff555555)); g.drawText(text, r.withTrimmedBottom(8), juce::Justification::centredBottom); return; }
        g.drawFittedText(text, r.toNearestInt().reduced(4, 0), juce::Justification::centred, 1, 0.7f);
    }
    void drawSvg(juce::Graphics& g, const pd::Svg& s)
    {
        if (s.env >= 0) { drawEnvelope(g, s); return; }
        static std::map<const char*, std::unique_ptr<juce::Drawable>> cache;
        auto it = cache.find(s.xml);
        if (it == cache.end())
        {
            std::unique_ptr<juce::Drawable> d;
            if (auto xml = juce::XmlDocument::parse(juce::String::fromUTF8(s.xml))) d = juce::Drawable::createFromSVG(*xml);
            it = cache.emplace(s.xml, std::move(d)).first;
        }
        if (it->second) it->second->drawWithin(g, juce::Rectangle<float>(s.x, s.y, s.w, s.h), juce::RectanglePlacement::stretchToFit, 1.0f);
    }
    void drawEnvelope(juce::Graphics& g, const pd::Svg& s)
    {
        const auto& ids = pd::envIds[s.env]; auto val = [&](const char* id) { auto* pa = paramFor(id); return pa ? pa->get() : 0; };
        const float a = val(ids.a) / 127.0f, d = val(ids.d) / 127.0f, su = val(ids.s) / 127.0f, r = val(ids.r) / 127.0f;
        const float W = s.w, H = s.h; const float ax = 3 + a * W * 0.30f, dx = ax + 3 + d * W * 0.30f, sy = 4 + (1 - su) * (H - 10), rx = W - 6 - r * W * 0.30f;
        juce::Path p; p.startNewSubPath(s.x, s.y + H - 2); p.lineTo(s.x + ax, s.y + 4); p.lineTo(s.x + juce::jmin(dx, rx), s.y + sy); p.lineTo(s.x + rx, s.y + sy); p.lineTo(s.x + W, s.y + H - 2);
        g.setColour(juce::Colour(0xfff2f5fa)); g.strokePath(p, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    juce::String describe(const juce::String& label, const char* pid) const
    {
        const int si = pid && pid[0] ? proc.findSlot(pid) : -1;
        if (si < 0) return label.trim() + ": shown as in the design. No documented MIDI control for it yet, so it is not sent to the device.";
        const auto& m = proc.mapRes.params[size_t(proc.slots[size_t(si)].map)];
        const juce::String addr = m.kind == ctl::Kind::CC ? "CC " + juce::String(m.a) : "NRPN " + juce::String(m.a) + ":" + juce::String(m.b);
        return juce::String(m.name) + ": " + addr + ", range " + juce::String(m.minV) + " to " + juce::String(m.maxV) + (m.rangeFromManual ? " (from the manual)" : " (assumed)") + (m.hwVerified ? ", tested on the device." : ", not tested on the device yet.");
    }
};

class CtlEditor : public juce::AudioProcessorEditor
{
public:
    explicit CtlEditor(CtlProcessor& p)
        : AudioProcessorEditor(p), proc(p),
          view(p, [this](int) { applyPageSize(); }, [this] { resetTo100(); }, [this](juce::Point<int>) { showInfo(); }),
          border(this, &constrainer), tooltips(this, 1000)
    {
        addAndMakeVisible(view); addAndMakeVisible(border); border.setBorderThickness(juce::BorderSize<int>(5));
        setConstrainer(&constrainer); setResizable(true, false);
        constrainer.setMinimumWidth(480); constrainer.setMaximumWidth(3600);
        const int w = int(ctled::kDesignW * ctled::kDefaultScale);
        setSize(w, int(std::round(view.designHeight() * w / ctled::kDesignW))); applyPageSize();
    }
    void resized() override { view.setBounds(getLocalBounds()); border.setBounds(getLocalBounds()); }
    void paint(juce::Graphics& g) override { g.fillAll(juce::Colours::black); }
private:
    void applyPageSize()
    {
        const double aspect = double(ctled::kDesignW) / double(view.designHeight());
        constrainer.setFixedAspectRatio(aspect);
        const int w = getWidth(); setSize(w, int(std::round(w / aspect)));
    }
    void resetTo100()
    {
        const int w = int(ctled::kDesignW * ctled::kDefaultScale); setSize(w, int(std::round(view.designHeight() * w / ctled::kDesignW)));
    }
    void showInfo()
    {
        juce::PopupMenu m; m.addItem(1, "Info bubbles", true, proc.getUi(ctled::kUiBubbles, 1) != 0); m.addItem(2, "About and support");
        m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this), [this](int r)
        {
            if (r == 1) proc.setUi(ctled::kUiBubbles, proc.getUi(ctled::kUiBubbles, 1) ? 0 : 1);
            if (r == 2) juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon, "Cobalt8M Controller",
                "Studio 87. Free and open source (AGPL-3.0).\n\n"
                "Values and CC numbers come from the maker's manual. Nothing is sent that is not in the map.\n"
                "Presets are the device's own; project data is saved with your DAW project.\nSupport: ko-fi.com/studio87");
        });
    }
    CtlProcessor& proc; MockupView view; juce::ComponentBoundsConstrainer constrainer; juce::ResizableBorderComponent border; juce::TooltipWindow tooltips;
};
}
