# Modal Cobalt8M: MIDI

Source tag for every CC row below: official: Modal COBALT8M manual v2 (https://modalelectronics.com/wp-content/uploads/2025/05/cobalt8M_MANUAL_v2.pdf, 42 pages), "MIDI CC Implemention Chart" (printed page 64) and the MIDI chapter (printed pages 62 and 63). This is the Cobalt8M's own manual. Every row below was compared with that chart on 2026-11-20. Every row is untested on hardware.

Earlier versions of this file were transcribed from the COBALT8 (non-M) manual v2; the Cobalt8M manual's chart gives the same numbers for all 104 mapped CCs. A dash means the chart lists no function for that CC.

Verification note (2026-11-20): all 104 CC rows in `map/params.csv` match the Cobalt8M chart by number and function name. The chart also lists messages that are not parameters and are not in the map: CC 0 Bank Change, CC 120 All Sound Off, CC 121 Reset All Controllers, CC 123 All Notes Off, CC 124 Omni Off, CC 125 Omni On. The chart has no value ranges, so ranges stay "0 to 127 assumed". No row has been captured from the owner's device, so none is hardware verified. Separate from the chart, the manual's MPE section says the Y-axis (front-back) is sent as CC 74; the chart itself shows no function at CC 74, so CC 74 is not mapped.

## CC chart
| CC | Function | Status |
|---|---|---|
| 0 | Bank Change | documented (chart, standard function); not in map/params.csv, bank change is sent by the preset buttons; untested |
| 1 | Mod Wheel | documented (chart); untested on hardware; range assumed 0 to 127 |
| 2 | - | not listed in chart, unmapped |
| 3 | LFO3 Rate | documented (chart); untested on hardware; range assumed 0 to 127 |
| 4 | - | not listed in chart, unmapped |
| 5 | Glide | documented (chart); untested on hardware; range assumed 0 to 127 |
| 6 | - | not listed in chart, unmapped |
| 7 | Main Volume | documented (chart); untested on hardware; range assumed 0 to 127 |
| 8 | (blank in chart) | not listed in chart, unmapped |
| 9 | Voice Mode | documented (chart); untested on hardware; range assumed 0 to 127 |
| 10 | - | not listed in chart, unmapped |
| 11 | Expression Pedal | documented (chart); untested on hardware; range assumed 0 to 127 |
| 12 | - | not listed in chart, unmapped |
| 13 | LFO3 Depth | documented (chart); untested on hardware; range assumed 0 to 127 |
| 14 | Osc1 Tune | documented (chart); untested on hardware; range assumed 0 to 127 |
| 15 | Osc1 Fine | documented (chart); untested on hardware; range assumed 0 to 127 |
| 16 | Osc1 Param A | documented (chart); untested on hardware; range assumed 0 to 127 |
| 17 | Osc2 Param A | documented (chart); untested on hardware; range assumed 0 to 127 |
| 18 | Osc Mix | documented (chart); untested on hardware; range assumed 0 to 127 |
| 19 | Osc1 Param B | documented (chart); untested on hardware; range assumed 0 to 127 |
| 20 | Osc2 Param B | documented (chart); untested on hardware; range assumed 0 to 127 |
| 21 | LFO3 Shape | documented (chart); untested on hardware; range assumed 0 to 127 |
| 22 | FEG Attack | documented (chart); untested on hardware; range assumed 0 to 127 |
| 23 | FEG Decay | documented (chart); untested on hardware; range assumed 0 to 127 |
| 24 | FEG Sustain | documented (chart); untested on hardware; range assumed 0 to 127 |
| 25 | FEG Release | documented (chart); untested on hardware; range assumed 0 to 127 |
| 26 | AEG Attack | documented (chart); untested on hardware; range assumed 0 to 127 |
| 27 | AEG Decay | documented (chart); untested on hardware; range assumed 0 to 127 |
| 28 | AEG Sustain | documented (chart); untested on hardware; range assumed 0 to 127 |
| 29 | AEG Release | documented (chart); untested on hardware; range assumed 0 to 127 |
| 30 | Osc2 Tune | documented (chart); untested on hardware; range assumed 0 to 127 |
| 31 | Osc2 Fine | documented (chart); untested on hardware; range assumed 0 to 127 |
| 32 | FEG Amount | documented (chart); untested on hardware; range assumed 0 to 127 |
| 33 | Morph | documented (chart); untested on hardware; range assumed 0 to 127 |
| 34 | Cutoff | documented (chart); untested on hardware; range assumed 0 to 127 |
| 35 | Resonance | documented (chart); untested on hardware; range assumed 0 to 127 |
| 36 | LFO1 Rate | documented (chart); untested on hardware; range assumed 0 to 127 |
| 37 | LFO1 Depth | documented (chart); untested on hardware; range assumed 0 to 127 |
| 38 | - | not listed in chart, unmapped |
| 39 | LFO1 Shape | documented (chart); untested on hardware; range assumed 0 to 127 |
| 40 | Octave | documented (chart); untested on hardware; range assumed 0 to 127 |
| 41 | Delay Mode | documented (chart); untested on hardware; range assumed 0 to 127 |
| 42 | Filter Type | documented (chart); untested on hardware; range assumed 0 to 127 |
| 43 | MEG Attack | documented (chart); untested on hardware; range assumed 0 to 127 |
| 44 | MEG Decay | documented (chart); untested on hardware; range assumed 0 to 127 |
| 45 | MEG Sustain | documented (chart); untested on hardware; range assumed 0 to 127 |
| 46 | MEG Release | documented (chart); untested on hardware; range assumed 0 to 127 |
| 47 | LFO2 Rate | documented (chart); untested on hardware; range assumed 0 to 127 |
| 48 | LFO2 Depth | documented (chart); untested on hardware; range assumed 0 to 127 |
| 49 | MEG Depth | documented (chart); untested on hardware; range assumed 0 to 127 |
| 50 | LFO2 Shape | documented (chart); untested on hardware; range assumed 0 to 127 |
| 51 | AEG Depth | documented (chart); untested on hardware; range assumed 0 to 127 |
| 52 | LFO1 MIDI Sync | documented (chart); untested on hardware; range assumed 0 to 127 |
| 53 | Arp Gate | documented (chart); untested on hardware; range assumed 0 to 127 |
| 54 | LFO2 MIDI Sync | documented (chart); untested on hardware; range assumed 0 to 127 |
| 55 | Delay MIDI Sync | documented (chart); untested on hardware; range assumed 0 to 127 |
| 56 | LFO1 Mode | documented (chart); untested on hardware; range assumed 0 to 127 |
| 57 | LFO2 Mode | documented (chart); untested on hardware; range assumed 0 to 127 |
| 58 | Arp Status | documented (chart); untested on hardware; range assumed 0 to 127 |
| 59 | Arp Octave | documented (chart); untested on hardware; range assumed 0 to 127 |
| 60 | Arp Direction | documented (chart); untested on hardware; range assumed 0 to 127 |
| 61 | Arp Division | documented (chart); untested on hardware; range assumed 0 to 127 |
| 62 | Osc1 Algorithm | documented (chart); untested on hardware; range assumed 0 to 127 |
| 63 | Osc2 Algorithm | documented (chart); untested on hardware; range assumed 0 to 127 |
| 64 | Sustain Pedal | documented (chart); untested on hardware; range assumed 0 to 127 |
| 65 | - | not listed in chart, unmapped |
| 66 | - | not listed in chart, unmapped |
| 67 | Seq Length | documented (chart); untested on hardware; range assumed 0 to 127 |
| 68 | Seq Mute | documented (chart); untested on hardware; range assumed 0 to 127 |
| 69 | Seq Clear | documented (chart); untested on hardware; range assumed 0 to 127 |
| 70 | Seq Hold | documented (chart); untested on hardware; range assumed 0 to 127 |
| 71 | Seq Loop | documented (chart); untested on hardware; range assumed 0 to 127 |
| 72 | Seq Record | documented (chart); untested on hardware; range assumed 0 to 127 |
| 73 | FX Amount | documented (chart); untested on hardware; range assumed 0 to 127 |
| 74 | - | not listed in chart, unmapped |
| 75 | Transpose | documented (chart); untested on hardware; range assumed 0 to 127 |
| 76 | - | not listed in chart, unmapped |
| 77 | - | not listed in chart, unmapped |
| 78 | Swing | documented (chart); untested on hardware; range assumed 0 to 127 |
| 79 | Seq Quantise | documented (chart); untested on hardware; range assumed 0 to 127 |
| 80 | AEG Type | documented (chart); untested on hardware; range assumed 0 to 127 |
| 81 | FEG Type | documented (chart); untested on hardware; range assumed 0 to 127 |
| 82 | MEG Type | documented (chart); untested on hardware; range assumed 0 to 127 |
| 83 | Audio In | documented (chart); untested on hardware; range assumed 0 to 127 |
| 84 | All EG Attack | documented (chart); untested on hardware; range assumed 0 to 127 |
| 85 | All EG Decay | documented (chart); untested on hardware; range assumed 0 to 127 |
| 86 | All EG Sustain | documented (chart); untested on hardware; range assumed 0 to 127 |
| 87 | All EG Release | documented (chart); untested on hardware; range assumed 0 to 127 |
| 88 | ModSlot 1 Depth | documented (chart); untested on hardware; range assumed 0 to 127 |
| 89 | ModSlot 2 Depth | documented (chart); untested on hardware; range assumed 0 to 127 |
| 90 | ModSlot 3 Depth | documented (chart); untested on hardware; range assumed 0 to 127 |
| 91 | ModSlot 4 Depth | documented (chart); untested on hardware; range assumed 0 to 127 |
| 92 | ModSlot 5 Depth | documented (chart); untested on hardware; range assumed 0 to 127 |
| 93 | ModSlot 6 Depth | documented (chart); untested on hardware; range assumed 0 to 127 |
| 94 | ModSlot 7 Depth | documented (chart); untested on hardware; range assumed 0 to 127 |
| 95 | ModSlot 8 Depth | documented (chart); untested on hardware; range assumed 0 to 127 |
| 96 | Chord Mode | documented (chart); untested on hardware; range assumed 0 to 127 |
| 97 | - | not listed in chart, unmapped |
| 98 | - | not listed in chart, unmapped |
| 99 | - | not listed in chart, unmapped |
| 100 | ModSlot 1 Source | documented (chart); untested on hardware; range assumed 0 to 127 |
| 101 | ModSlot 2 Source | documented (chart); untested on hardware; range assumed 0 to 127 |
| 102 | ModSlot 3 Source | documented (chart); untested on hardware; range assumed 0 to 127 |
| 103 | ModSlot 4 Source | documented (chart); untested on hardware; range assumed 0 to 127 |
| 104 | ModSlot 5 Source | documented (chart); untested on hardware; range assumed 0 to 127 |
| 105 | ModSlot 6 Source | documented (chart); untested on hardware; range assumed 0 to 127 |
| 106 | ModSlot 7 Source | documented (chart); untested on hardware; range assumed 0 to 127 |
| 107 | ModSlot 8 Source | documented (chart); untested on hardware; range assumed 0 to 127 |
| 108 | ModSlot 1 Dest | documented (chart); untested on hardware; range assumed 0 to 127 |
| 109 | ModSlot 2 Dest | documented (chart); untested on hardware; range assumed 0 to 127 |
| 110 | ModSlot 3 Dest | documented (chart); untested on hardware; range assumed 0 to 127 |
| 111 | ModSlot 4 Dest | documented (chart); untested on hardware; range assumed 0 to 127 |
| 112 | ModSlot 5 Dest | documented (chart); untested on hardware; range assumed 0 to 127 |
| 113 | ModSlot 6 Dest | documented (chart); untested on hardware; range assumed 0 to 127 |
| 114 | ModSlot 7 Dest | documented (chart); untested on hardware; range assumed 0 to 127 |
| 115 | ModSlot 8 Dest | documented (chart); untested on hardware; range assumed 0 to 127 |
| 116 | Joystick X+ | documented (chart); untested on hardware; range assumed 0 to 127 |
| 117 | Joystick X- | documented (chart); untested on hardware; range assumed 0 to 127 |
| 118 | Joystick Y- | documented (chart); untested on hardware; range assumed 0 to 127 |
| 119 | Patch Gain | documented (chart); untested on hardware; range assumed 0 to 127 |
| 120 | All Sound Off | documented (chart, standard function); not in map/params.csv, bank change is sent by the preset buttons; untested |
| 121 | Reset All Controllers | documented (chart, standard function); not in map/params.csv, bank change is sent by the preset buttons; untested |
| 122 | - | not listed in chart, unmapped |
| 123 | All Notes Off | documented (chart, standard function); not in map/params.csv, bank change is sent by the preset buttons; untested |
| 124 | Omni Off | documented (chart, standard function); not in map/params.csv, bank change is sent by the preset buttons; untested |
| 125 | Omni On | documented (chart, standard function); not in map/params.csv, bank change is sent by the preset buttons; untested |
| 126 | - | not listed in chart, unmapped |
| 127 | - | not listed in chart, unmapped |

Notes on the chart:
- The printed chart has no entry for Joystick Y+ (only X+, X- and Y-). The MPE section says Y is sent as CC 74, but that is not a chart row; test on the unit.
- CC 8 is blank in the printed chart (no dash, no name).
- The manual gives no value ranges or scaling per CC. Capture them from the unit.

## MIDI behaviour (official: COBALT8M manual v2, MIDI chapter; untested on hardware)
- Channel: set in the on-screen settings or in MODALapp.
- All encoders send the matching CC when changed (depends on SHIFT or panel state). Keys send notes for their octave and scale.
- The keyboard sends channel aftertouch; the joystick can send pitch bend.
- Program change: send a bank change (0 to 4) then a program change (0 to 99) to load a preset.
- Clock: with Auto, it follows incoming MIDI clock; with none it uses its internal tempo. Sync in/out is 3.3 V, rising edge, 1 pulse per 16th note, never above 5 V. Sync source priority: Sync, then external MIDI, then internal; fallback after a 3 second timeout.
- Soft thru (set in the app): DIN Thru forwards DIN in to DIN out; USB Thru forwards DIN in to USB out and USB in to DIN out.
- DIN filters (set in the app) on input and output: Notes, CC, Program Change, Aftertouch, Pitchbend, Transport, Clock, Sysex. Default: DIN in allows everything; DIN out allows everything except clock.
- MIDI In Omni: in the Settings pages or the MODALapp Settings tab.
- Settings pages list: Channel, Omni, Pitchbend Range, MPE Mode, MPE Master Channel, MPE Num Channels, MPE Pitchbend Range, DIN In/Out Filters, DIN Thru, USB Thru, Arp/Seq Out, MIDI-in Oct Offset, MIDI-in Monitor.

## Patch dump (UNVERIFIED, not from the manual)
Source: UNVERIFIED. We have no capture file in this repo, so this cannot be called an owner capture. The notes say: patch dump is 137 bytes with header `F0 00 21 07 19 02 04 [slot]`. Still to capture: patch request, patch name, factory reset, and the FX SysEx details. Treat as unverified until the capture files are checked.

## Screen map (parameter groups)
Patch preset (load, save, init, randomise, linked sequence); Keyboard/Voice (voice mode, glide, octave, transpose, chord latch, vintage, voice width, chord invert, sustain, velocity, aftertouch, note and expression assigns with depths); Osc1 and Osc2 (algorithm, param A, param B, tune, fine); Osc general (mix, drift, free run); FX presets, FX-1 to FX-3 (type A to F, delay mode, delay sync, swap) and level; Arpeggiator (status, division, direction, octave, swing, gate); Sequencer (play, record, mode, length, step length, number of steps, step mode, overdub, mute, clear, four animation lanes, loop, hold); Clock (tempo); Audio (main volume, patch gain, headphone level, gain boost, audio in volume and to FX); Filter (cutoff, reso, morph, type, cutoff scaling); Filter, Amp and Mod envelopes (amount/depth, ADSR, type, assign); LFO1 to LFO3 (shape, rate, depth, mode, sync, assign); 8 mod slots (status, source, destination, depth, clear); Mod general (note to cutoff, Y+ to LFO1 depth, aftertouch to cutoff, velocity to amp EG amount); Joystick (XY lock, pitch bend status, X+/X-/Y+/Y- assign and depth).


## NRPN
No NRPN is documented in the manual pages we used, and the plugin sends none. Do not add NRPN entries without an official document or an owner capture.
