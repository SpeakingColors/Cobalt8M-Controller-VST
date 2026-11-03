#!/usr/bin/env python3
"""Reads the six mockup pages in docs/mockups with a headless browser and writes src/PageData.h:
every box, text, knob, button and graph with its position, colours and size, in painting order.
The JUCE editor draws that data natively (no web view). Control to parameter bindings are the BIND tables below,
and only list controls that have a documented CC in map/params.csv. Re-run after a mockup changes:
    python3 tools/port_mockups.py            (needs: pip install playwright; playwright install chromium)
"""
import asyncio, json, os, re, sys
from playwright.async_api import async_playwright

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PAGES = ["CobaltPresets", "Cobalt", "CobaltFX", "CobaltSeq", "CobaltKeys", "CobaltSettings"]
PAGE_TITLES = ["Preset Manager", "Modulation", "FX", "Sequencer", "Keyboard", "Settings"]

JS = r"""
() => {
  const items = []; const groups = new Map(); let gid = 0;
  const root = document.querySelector('x-dc') || document.body;
  const wood = document.querySelector('.wood-cheek'); const o = wood.getBoundingClientRect();
  const ox = o.left + window.scrollX, oy = o.top + window.scrollY;
  const R = el => { const r = el.getBoundingClientRect(); return [r.left + window.scrollX - ox, r.top + window.scrollY - oy, r.width, r.height]; };
  const clear = c => !c || c === 'rgba(0, 0, 0, 0)' || c === 'transparent';
  for (const el of root.querySelectorAll('*')) {
    const tag = el.tagName.toLowerCase();
    if (el.closest('svg') && tag !== 'svg') continue;
    const cs = getComputedStyle(el);
    if (cs.position === 'fixed') continue;
    if (el.closest('[style*="position: fixed"]')) continue;
    if (tag === 'svg') {
      const circles = el.querySelectorAll('circle');
      const isKnob = circles.length >= 2 && !!el.querySelector('text') && [...circles].some(c => c.getAttribute('fill') === 'none');
      const lab = el.getAttribute('aria-label') || '';
      if (isKnob) {
        const arc = el.querySelector('path'); const t = el.querySelector('text'); const vb = (el.getAttribute('viewBox') || '0 0 40 40').split(' ').map(Number);
        items.push({k: 'knob', r: R(el), label: lab, track: circles[0].getAttribute('stroke'), arc: arc ? arc.getAttribute('stroke') : '#fff', fg: t.getAttribute('fill'), vb: vb[2], txt: t.textContent});
      } else {
        el.setAttribute('xmlns', 'http://www.w3.org/2000/svg');
        items.push({k: 'svg', r: R(el), label: lab, html: el.outerHTML});
      }
      continue;
    }
    if (tag === 'button') {
      if (!groups.has(el.parentElement)) groups.set(el.parentElement, gid++);
      items.push({k: 'button', r: R(el), text: el.textContent.trim(), bg: cs.backgroundColor, fg: cs.color, fs: parseFloat(cs.fontSize), fw: cs.fontWeight, rad: cs.borderTopLeftRadius, bd: cs.borderTopColor, bdw: parseFloat(cs.borderTopWidth), g: groups.get(el.parentElement)});
      continue;
    }
    if (el.closest('button')) continue;
    if ((tag === 'input' || tag === 'textarea') && el.placeholder) {
      const r0 = R(el); items.push({k: 'text', r: [r0[0] + 12, r0[1] + r0[3] / 2 - 9, r0[2] - 24, 18], text: el.placeholder, fg: 'rgba(184, 203, 230, 1)', fs: parseFloat(cs.fontSize) || 13, fw: '400', mono: false, ls: 0});
    }
    if (tag === 'a') items.push({k: 'link', r: R(el), href: el.getAttribute('href') || ''});
    const bg = cs.backgroundColor, bgi = cs.backgroundImage;
    if (!el.classList.contains('wood-cheek') && (!clear(bg) || (bgi && bgi !== 'none'))) {
      items.push({k: 'box', r: R(el), bg, bgi: bgi === 'none' ? '' : bgi.slice(0, 240), rad: cs.borderTopLeftRadius,
        b: [parseFloat(cs.borderTopWidth), parseFloat(cs.borderRightWidth), parseFloat(cs.borderBottomWidth), parseFloat(cs.borderLeftWidth)],
        bc: [cs.borderTopColor, cs.borderRightColor, cs.borderBottomColor, cs.borderLeftColor]});
    } else if (!el.classList.contains('wood-cheek')) {
      const b = [parseFloat(cs.borderTopWidth), parseFloat(cs.borderRightWidth), parseFloat(cs.borderBottomWidth), parseFloat(cs.borderLeftWidth)];
      if (b.some(x => x > 0)) items.push({k: 'box', r: R(el), bg: 'rgba(0, 0, 0, 0)', bgi: '', rad: cs.borderTopLeftRadius, b, bc: [cs.borderTopColor, cs.borderRightColor, cs.borderBottomColor, cs.borderLeftColor]});
    }
    let txt = '';
    for (const n of el.childNodes) if (n.nodeType === 3) txt += n.textContent;
    txt = txt.replace(/\s+/g, ' ').trim();
    if (txt) {
      const range = document.createRange(); range.selectNodeContents(el);
      let l = 1e9, t = 1e9, rr = -1e9, b = -1e9;
      for (const q of range.getClientRects()) { l = Math.min(l, q.left); t = Math.min(t, q.top); rr = Math.max(rr, q.right); b = Math.max(b, q.bottom); }
      items.push({k: 'text', r: [l + window.scrollX - ox, t + window.scrollY - oy, rr - l, b - t], text: txt, fg: cs.color, fs: parseFloat(cs.fontSize), fw: cs.fontWeight, mono: cs.fontFamily.includes('Mono'), ls: parseFloat(cs.letterSpacing) || 0});
    }
  }
  const body = wood.parentElement.getBoundingClientRect();
  return {items, size: [body.width, body.height]};
}
"""

# ---- bindings: (page index or -1 for the shared top part) -> list of (x, y, parameter id). Only documented CCs.
# Positions are mockup pixels of the control's top left corner (as read from the page).
KNOB_BIND = {
 (103,264):'cc_5',
 (236,253):'cc_16',(347,253):'cc_19',(458,253):'cc_18',(570,253):'cc_17',(681,253):'cc_20',
 (236,324):'cc_14',(347,324):'cc_15',(570,324):'cc_30',(681,324):'cc_31',
 (793,227):'cc_34',(817,359):'cc_35',(927,359):'cc_33',
 (1428,140):'cc_32',(1175,203):'cc_22',(1290,203):'cc_23',(1405,203):'cc_24',(1520,203):'cc_25',
 (1428,281):'cc_51',(1175,344):'cc_26',(1290,344):'cc_27',(1405,344):'cc_28',(1520,344):'cc_29',
 # mod matrix, slots 1 to 8 (depth)
 (92,530):'cc_88',(222,530):'cc_89',(352,530):'cc_90',(482,530):'cc_91',(612,530):'cc_92',(742,530):'cc_93',(872,530):'cc_94',(1002,530):'cc_95',
}
PAGE_KNOB_BIND = {
 1: {(317,733):'cc_39',(421,733):'cc_36',(525,733):'cc_37',(650,733):'cc_50',(754,733):'cc_47',(858,733):'cc_48',
     (983,733):'cc_21',(1087,733):'cc_3',(1191,733):'cc_13',
     (1297,711):'cc_49',(1361,711):'cc_43',(1425,711):'cc_44',(1489,711):'cc_45',(1553,711):'cc_46'},
 2: {(565,998):'cc_73'},
 3: {(136,886):'cc_67'},
 4: {(74,983):'cc_61',(138,983):'cc_59',(202,983):'cc_78',(266,983):'cc_53',(397,939):'cc_75',(463,939):'cc_40'},
}
# envelopes drawn live from their four knobs: (x, y of the svg) -> (attack, decay, sustain, release) parameter ids
ENV_BIND = {(1182,140):('cc_22','cc_23','cc_24','cc_25'),(1182,281):('cc_26','cc_27','cc_28','cc_29'),(1319,655):('cc_43','cc_44','cc_45','cc_46')}
# buttons: (page, text, x) -> (parameter id, value when on, value when off, kind) kind: toggle | set | momentary
BTN_BIND = {
 (3,'Hold'):('cc_70',127,0,'toggle'),(3,'Loop'):('cc_71',127,0,'toggle'),(3,'Clear'):('cc_69',127,0,'momentary'),
 (4,'Chord mode'):('cc_96',127,0,'toggle'),
 (-1,'● Record'):('cc_72',127,0,'toggle'),
}
SLIDER_BIND = {'Volume':'cc_7','Patch Gain':'cc_119'}

def rgba(s):
    m = re.match(r'rgba?\(([\d.]+),\s*([\d.]+),\s*([\d.]+)(?:,\s*([\d.]+))?\)', s or '')
    if m:
        r, g, b = (int(float(m.group(i))) for i in (1, 2, 3)); a = float(m.group(4)) if m.group(4) is not None else 1.0
        return (int(round(a * 255)) << 24) | (r << 16) | (g << 8) | b
    m = re.match(r'#([0-9a-fA-F]{6})', s or '')
    if m: return 0xFF000000 | int(m.group(1), 16)
    return 0xFFFFFFFF

def px(s):
    m = re.match(r'([\d.]+)(px|%)?', s or '0'); return (float(m.group(1)), m.group(2)) if m else (0.0, None)

def cstr(s):
    # ASCII only in the generated file (non-ASCII bytes as octal escapes), so every compiler reads it the same way
    out = []
    for ch in s.replace('\n', ' ').encode('utf-8'):
        c = chr(ch)
        if c == '\\': out.append('\\\\')
        elif c == '"': out.append('\\"')
        elif ch < 32 or ch > 126: out.append('\\%03o' % ch)
        else: out.append(c)
    return '"' + ''.join(out) + '"'

def grad(bgi):
    m = re.match(r'linear-gradient\(([^,]+),(.*)\)$', bgi)
    if not m or 'repeating' in bgi: return None
    cols = re.findall(r'rgba?\([^)]*\)', m.group(2))
    if len(cols) < 2: return None
    ang = m.group(1).strip()
    return (rgba(cols[0]), rgba(cols[-1]), 2 if '180' in ang or 'bottom' in ang else 1)

async def extract():
    out = {}
    async with async_playwright() as p:
        b = await p.chromium.launch(); pg = await b.new_page(viewport={'width': 1800, 'height': 1500})
        for n in PAGES:
            await pg.goto('file://' + os.path.join(ROOT, 'docs', 'mockups', n + '.dc.html')); await pg.wait_for_timeout(1000)
            out[n] = await pg.evaluate(JS)
        await b.close()
    return out

def gen(data):
    L = []; L.append('// GENERATED by tools/port_mockups.py from docs/mockups. Do not edit by hand; edit the mockup or the script and re-run.')
    L.append('#pragma once\n#include <cstdint>\nnamespace pd\n{')
    L.append('struct Box { float x, y, w, h; uint32_t c, c2; float rad; int dir; };')
    L.append('struct Text { float x, y, w, h; const char* s; uint32_t c; float fs; int bold, mono; float ls; };')
    L.append('struct Knob { float x, y, size; const char* label; const char* param; int init; const char* text0; uint32_t track, arc, fg; int key; };')
    L.append('struct Btn { float x, y, w, h; const char* text; uint32_t bg, fg, bd; float bdw, fs, rad; int group, sel, key; const char* param; int onv, offv; int kind; int act; uint32_t selbg; };')
    L.append('struct Svg { float x, y, w, h; const char* xml; int env; };')
    L.append('struct Link { float x, y, w, h; int page; };')
    L.append('struct EnvIds { const char* a; const char* d; const char* s; const char* r; };')
    L.append('struct Item { char k; int i; };')
    L.append('struct Page { const char* name; float w, h; const Box* boxes; const Text* texts; const Knob* knobs; const Btn* btns; const Svg* svgs; const Link* links; const Item* items; int nItems, nLinks; };')
    L.append('enum Act { ActNone, ActReset100, ActInfo, ActSettings, ActPcPrev, ActPcNext, ActBank, ActKey, ActSlider, ActPreset };')
    L.append('constexpr EnvIds envIds[] = {')
    for k, v in ENV_BIND.items(): L.append('  { %s },' % ', '.join(cstr(x) for x in v))
    L.append('};')
    pagedefs = []; keyc = 0; keymap = {}
    def key_for(page, kind, x, y, label):
        nonlocal keyc
        shared = y < 595 and page >= 0
        kk = (('S' if shared else str(page)), kind, round(x), round(y))
        if kk not in keymap: keymap[kk] = keyc; keyc += 1
        return keymap[kk]
    for pi, n in enumerate(PAGES):
        d = data[n]; items = d['items']; boxes = []; texts = []; knobs = []; btns = []; svgs = []; links = []; order = []
        for it in items:
            k = it['k']; x, y, w, h = it['r']
            if w <= 0 or h <= 0 and k != 'box': continue
            if k == 'box':
                if w < 0.5 or h < 0.5: continue
                rad, unit = px(it['rad']); rad = min(w, h) / 2 if unit == '%' else rad
                g = grad(it['bgi']) if it['bgi'] else None
                if it['bgi'] and not g and 'repeating' in it['bgi']: pass
                a = rgba(it['bg'])
                if g: c, c2, dr = g
                else: c, c2, dr = a, a, 0
                if (c >> 24) > 0 or g:
                    boxes.append((x, y, w, h, c, c2, rad, dr)); order.append(('b', len(boxes) - 1))
                bw = it['b']; bc = it['bc']
                if any(v > 0 for v in bw):
                    # equal borders on a rounded box are drawn as an outline; single sided borders as thin strips
                    if min(bw) > 0 and len(set(bc)) == 1:
                        boxes.append((x, y, w, h, 0x00000000 | (rgba(bc[0]) & 0xFFFFFF), rgba(bc[0]), rad, 3)); order.append(('b', len(boxes) - 1))
                    else:
                        if bw[0] > 0: boxes.append((x, y, w, bw[0], rgba(bc[0]), rgba(bc[0]), 0, 0)); order.append(('b', len(boxes) - 1))
                        if bw[2] > 0: boxes.append((x, y + h - bw[2], w, bw[2], rgba(bc[2]), rgba(bc[2]), 0, 0)); order.append(('b', len(boxes) - 1))
                        if bw[1] > 0: boxes.append((x + w - bw[1], y, bw[1], h, rgba(bc[1]), rgba(bc[1]), 0, 0)); order.append(('b', len(boxes) - 1))
                        if bw[3] > 0: boxes.append((x, y, bw[3], h, rgba(bc[3]), rgba(bc[3]), 0, 0)); order.append(('b', len(boxes) - 1))
            elif k == 'text':
                texts.append((x, y, w, h, it['text'], rgba(it['fg']), it['fs'], 1 if int(it['fw']) >= 600 else 0, 1 if it['mono'] else 0, it['ls'])); order.append(('t', len(texts) - 1))
            elif k == 'knob':
                lab = it['label']; m = re.match(r'^(.*?)\s*(-?\d+)$', lab); init = int(m.group(2)) if m else 0
                name = m.group(1) if m else lab
                pid = PAGE_KNOB_BIND.get(pi, {}).get((round(x), round(y)))
                if pid is None and y < 595: pid = KNOB_BIND.get((round(x), round(y)))
                txt0 = it['txt'] if not re.match(r'^-?\d+$', it['txt'] or '') else ''
                knobs.append((x, y, w, name, pid or '', init, txt0, rgba(it['track']), rgba(it['arc']), rgba(it['fg']), key_for(pi, 'k', x, y, name))); order.append(('k', len(knobs) - 1))
            elif k == 'button':
                t = it['text']; bg = rgba(it['bg']); act = 0; param = ''; onv = 127; offv = 0; kind = 0
                if t.startswith('⤡'): act = 1
                elif t == 'i' and x > 1500: act = 2
                elif t == '⚙': act = 3
                elif t == '−' and round(x) == 237: act = 4
                elif t == '+' and round(x) == 274: act = 5
                elif pi == 0 and re.match(r'^Bank \d$', t): act = 6
                elif pi == 4 and abs(y - 715) < 4 and h > 100:
                    act = 7   # piano key: note number goes in offv (C3 = 48)
                    whites = sorted(i2['r'][0] for i2 in items if i2['k'] == 'button' and abs(i2['r'][1] - 715) < 4 and i2['r'][3] > 150)
                    if h > 150 and x in whites: offv = (whites.index(x) // 7) * 12 + [0, 2, 4, 5, 7, 9, 11][whites.index(x) % 7] + 48
                    else:
                        j = max(i for i in range(len(whites)) if whites[i] <= x + 1)
                        offv = (j // 7) * 12 + [0, 2, 4, 5, 7, 9, 11][j % 7] + 48 + 1
                for s, pid in SLIDER_BIND.items():
                    if t.startswith(s): act = 8; param = pid; t = s
                bb = BTN_BIND.get((pi, t)) or BTN_BIND.get((-1, t))
                if bb: param, onv, offv, kk = bb; kind = {'toggle': 1, 'set': 2, 'momentary': 3}[kk]
                btns.append((x, y, w, h, t, bg, rgba(it['fg']), rgba(it['bd']), it['bdw'], it['fs'], px(it['rad'])[0] if px(it['rad'])[1] != '%' else min(w, h) / 2, it['g'], 0, key_for(pi, 'b', x, y, t), param, onv, offv, kind, act)); order.append(('n', len(btns) - 1))
            elif k == 'svg':
                env = -1
                if it['label'] == 'Envelope shape':
                    for ei, kk2 in enumerate(ENV_BIND):
                        if abs(kk2[0] - x) < 3 and abs(kk2[1] - y) < 3: env = ei
                svgs.append((x, y, w, h, it['html'], env)); order.append(('s', len(svgs) - 1))
            elif k == 'link':
                m = re.match(r'(Cobalt\w*)\.dc\.html', it['href'])
                if m and m.group(1) in PAGES: links.append((x, y, w, h, PAGES.index(m.group(1))))
        # button selected state: in a group with more than one button, the one whose background differs from the others
        from collections import Counter
        bygroup = {}
        for i, b in enumerate(btns): bygroup.setdefault(b[11], []).append(i)
        sel = {}; selbg = {}; normbg = {}
        for gidx, idxs in bygroup.items():
            cols = Counter(btns[i][5] for i in idxs); common = cols.most_common(1)[0][0]
            others = [c for c in cols if c != common]
            for i in idxs:
                normbg[i] = common if len(cols) > 1 else btns[i][5]
                selbg[i] = others[0] if others else 0xFF141416
                is_sel = btns[i][5] != common if len(idxs) > 1 and len(cols) > 1 else (((btns[i][5] >> 24) & 255) > 200 and ((btns[i][5] >> 16 & 255) + (btns[i][5] >> 8 & 255) + (btns[i][5] & 255)) < 90)
                sel[i] = 1 if is_sel else 0
                if btns[i][18] == 7:
                    sel[i] = 0; selbg[i] = 0xFFFFC857
                    white = btns[i][3] > 150
                    cls = [j for j in idxs if btns[j][18] == 7 and (btns[j][3] > 150) == white]
                    normbg[i] = Counter(btns[j][5] for j in cls).most_common(1)[0][0]
                if len(idxs) > 1 and len(cols) > 1 and btns[i][18] != 7 and sel[i] == 0 and False: pass
        L.append(f'// ---- page {n}')
        L.append(f'constexpr Box boxes{pi}[] = {{' + ','.join('{%g,%g,%g,%g,0x%08Xu,0x%08Xu,%g,%d}' % b for b in boxes) + '};')
        L.append(f'constexpr Text texts{pi}[] = {{' + ','.join('{%g,%g,%g,%g,%s,0x%08Xu,%g,%d,%d,%g}' % (b[0], b[1], b[2], b[3], cstr(b[4]), b[5], b[6], b[7], b[8], b[9]) for b in texts) + '};')
        L.append(f'constexpr Knob knobs{pi}[] = {{' + ','.join('{%g,%g,%g,%s,%s,%d,%s,0x%08Xu,0x%08Xu,0x%08Xu,%d}' % (b[0], b[1], b[2], cstr(b[3]), cstr(b[4]), b[5], cstr(b[6]), b[7], b[8], b[9], b[10]) for b in knobs) + ('{0,0,0,"","",0,"",0,0,0,0}' if not knobs else '') + '};')
        L.append(f'constexpr Btn btns{pi}[] = {{' + ','.join('{%g,%g,%g,%g,%s,0x%08Xu,0x%08Xu,0x%08Xu,%g,%g,%g,%d,%d,%d,%s,%d,%d,%d,%d,0x%08Xu}' % (b[0], b[1], b[2], b[3], cstr(b[4]), normbg[i], b[6], b[7], b[8], b[9], b[10], b[11], sel[i], b[13], cstr(b[14]), b[15], b[16], b[17], b[18], selbg[i]) for i, b in enumerate(btns)) + ('{0,0,0,0,"",0,0,0,0,0,0,0,0,0,"",0,0,0,0,0}' if not btns else '') + '};')
        L.append(f'constexpr Svg svgs{pi}[] = {{' + ','.join('{%g,%g,%g,%g,%s,%d}' % (b[0], b[1], b[2], b[3], cstr(b[4]), b[5]) for b in svgs) + ('{0,0,0,0,"",-1}' if not svgs else '') + '};')
        L.append(f'constexpr Link links{pi}[] = {{' + ','.join('{%g,%g,%g,%g,%d}' % b for b in links) + ('{0,0,0,0,0}' if not links else '') + '};')
        L.append(f'constexpr Item items{pi}[] = {{' + ','.join("{'%s',%d}" % o for o in order) + '};')
        pagedefs.append(f'{{ {cstr(PAGE_TITLES[pi])}, {round(d["size"][0])}, {round(d["size"][1])}, boxes{pi}, texts{pi}, knobs{pi}, btns{pi}, svgs{pi}, links{pi}, items{pi}, {len(order)}, {len(links)} }}')
    L.append('constexpr Page pages[] = {\n  ' + ',\n  '.join(pagedefs) + '\n};')
    L.append(f'constexpr int kNumKeys = {keyc};')
    L.append('}')
    return '\n'.join(L) + '\n'

if __name__ == '__main__':
    data = asyncio.run(extract())
    open(os.path.join(ROOT, 'src', 'PageData.h'), 'w').write(gen(data))
    print('wrote src/PageData.h')
