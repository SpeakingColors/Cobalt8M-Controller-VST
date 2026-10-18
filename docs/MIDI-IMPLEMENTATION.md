# Modal Cobalt8M: MIDI

Source: COBALT8 manual v2, "MIDI" chapter and "MIDI CC Implementation Chart" (pages 62 to 65), plus the screen map (pages 20 and 21). A dash means the chart lists no function for that CC. Check against the real unit before relying on it in code.

## CC chart
| CC | Function |
|---|---|
| 0 | Bank Change |
| 1 | Mod Wheel |
| 2 | - |
| 3 | LFO3 Rate |
| 4 | - |
| 5 | Glide |
| 6 | - |
| 7 | Main Volume |
| 8 | (blank in chart) |
| 9 | Voice Mode |
| 10 | - |
| 11 | Expression Pedal |
| 12 | - |
| 13 | LFO3 Depth |
| 14 | Osc1 Tune |
| 15 | Osc1 Fine |
| 16 | Osc1 Param A |
| 17 | Osc2 Param A |
| 18 | Osc Mix |
| 19 | Osc1 Param B |
| 20 | Osc2 Param B |
| 21 | LFO3 Shape |
| 22 | FEG Attack |
| 23 | FEG Decay |
| 24 | FEG Sustain |
| 25 | FEG Release |
| 26 | AEG Attack |
| 27 | AEG Decay |
| 28 | AEG Sustain |
| 29 | AEG Release |
| 30 | Osc2 Tune |
| 31 | Osc2 Fine |
| 32 | FEG Amount |
| 33 | Morph |
| 34 | Cutoff |
| 35 | Resonance |
| 36 | LFO1 Rate |
| 37 | LFO1 Depth |
| 38 | - |
| 39 | LFO1 Shape |
| 40 | Octave |
| 41 | Delay Mode |
| 42 | Filter Type |
| 43 | MEG Attack |
| 44 | MEG Decay |
| 45 | MEG Sustain |
| 46 | MEG Release |
| 47 | LFO2 Rate |
| 48 | LFO2 Depth |
| 49 | MEG Depth |
| 50 | LFO2 Shape |
| 51 | AEG Depth |
| 52 | LFO1 MIDI Sync |
| 53 | Arp Gate |
| 54 | LFO2 MIDI Sync |
| 55 | Delay MIDI Sync |
| 56 | LFO1 Mode |
| 57 | LFO2 Mode |
| 58 | Arp Status |
| 59 | Arp Octave |
| 60 | Arp Direction |
| 61 | Arp Division |
| 62 | Osc1 Algorithm |
| 63 | Osc2 Algorithm |
| 64 | Sustain Pedal |
| 65 | - |
| 66 | - |
| 67 | Seq Length |
| 68 | Seq Mute |
| 69 | Seq Clear |
| 70 | Seq Hold |
| 71 | Seq Loop |
| 72 | Seq Record |
| 73 | FX Amount |
| 74 | - |
| 75 | Transpose |
| 76 | - |
| 77 | - |
| 78 | Swing |
| 79 | Seq Quantise |
| 80 | AEG Type |
| 81 | FEG Type |
| 82 | MEG Type |
| 83 | Audio In |
| 84 | All EG Attack |
| 85 | All EG Decay |
| 86 | All EG Sustain |
| 87 | All EG Release |
| 88 | ModSlot 1 Depth |
| 89 | ModSlot 2 Depth |
| 90 | ModSlot 3 Depth |
| 91 | ModSlot 4 Depth |
| 92 | ModSlot 5 Depth |
| 93 | ModSlot 6 Depth |
| 94 | ModSlot 7 Depth |
| 95 | ModSlot 8 Depth |
| 96 | Chord Mode |
| 97 | - |
| 98 | - |
| 99 | - |
| 100 | ModSlot 1 Source |
| 101 | ModSlot 2 Source |
| 102 | ModSlot 3 Source |
| 103 | ModSlot 4 Source |
| 104 | ModSlot 5 Source |
| 105 | ModSlot 6 Source |
| 106 | ModSlot 7 Source |
| 107 | ModSlot 8 Source |
| 108 | ModSlot 1 Dest |
| 109 | ModSlot 2 Dest |
| 110 | ModSlot 3 Dest |
| 111 | ModSlot 4 Dest |
| 112 | ModSlot 5 Dest |
| 113 | ModSlot 6 Dest |
| 114 | ModSlot 7 Dest |
| 115 | ModSlot 8 Dest |
| 116 | Joystick X+ |
| 117 | Joystick X- |
| 118 | Joystick Y- |
| 119 | Patch Gain |
| 120 | All Sound Off |
| 121 | Reset All Controllers |
| 122 | - |
| 123 | All Notes Off |
| 124 | Omni Off |
| 125 | Omni On |
| 126 | - |
| 127 | - |

Notes on the chart:
- The printed chart has no entry for Joystick Y+ (only X+, X- and Y-). Test on the unit.
- CC 8 is blank in the printed chart (no dash, no name).
- The manual gives no value ranges or scaling per CC. Capture them from the unit.

## MIDI behaviour (from the manual)
- Channel: set in the on-screen settings or in MODALapp.
- All encoders send the matching CC when changed (depends on SHIFT or panel state). Keys send notes for their octave and scale.
- The keyboard sends channel aftertouch; the joystick can send pitch bend.
- Program change: send a bank change (0 to 4) then a program change (0 to 99) to load a preset.
- Clock: with Auto, it follows incoming MIDI clock; with none it uses its internal tempo. Sync in/out is 3.3 V, rising edge, 1 pulse per 16th note, never above 5 V. Sync source priority: Sync, then external MIDI, then internal; fallback after a 3 second timeout.
- Soft thru (set in the app): DIN Thru forwards DIN in to DIN out; USB Thru forwards DIN in to USB out and USB in to DIN out.
- DIN filters (set in the app) on input and output: Notes, CC, Program Change, Aftertouch, Pitchbend, Transport, Clock, Sysex. Default: DIN in allows everything; DIN out allows everything except clock.
- MIDI In Omni: in the Settings pages or the MODALapp Settings tab.
- Settings pages list: Channel, Omni, Pitchbend Range, MPE Mode, MPE Master Channel, MPE Num Channels, MPE Pitchbend Range, DIN In/Out Filters, DIN Thru, USB Thru, Arp/Seq Out, MIDI-in Oct Offset, MIDI-in Monitor.

## Patch dump (not from the manual)
From our own capture notes: patch dump is 137 bytes with header `F0 00 21 07 19 02 04 [slot]`. Still to capture: patch request, patch name, factory reset, and the FX SysEx details. Treat as unverified until the capture files are checked.

## Screen map (parameter groups)
Patch preset (load, save, init, randomise, linked sequence); Keyboard/Voice (voice mode, glide, octave, transpose, chord latch, vintage, voice width, chord invert, sustain, velocity, aftertouch, note and expression assigns with depths); Osc1 and Osc2 (algorithm, param A, param B, tune, fine); Osc general (mix, drift, free run); FX presets, FX-1 to FX-3 (type A to F, delay mode, delay sync, swap) and level; Arpeggiator (status, division, direction, octave, swing, gate); Sequencer (play, record, mode, length, step length, number of steps, step mode, overdub, mute, clear, four animation lanes, loop, hold); Clock (tempo); Audio (main volume, patch gain, headphone level, gain boost, audio in volume and to FX); Filter (cutoff, reso, morph, type, cutoff scaling); Filter, Amp and Mod envelopes (amount/depth, ADSR, type, assign); LFO1 to LFO3 (shape, rate, depth, mode, sync, assign); 8 mod slots (status, source, destination, depth, clear); Mod general (note to cutoff, Y+ to LFO1 depth, aftertouch to cutoff, velocity to amp EG amount); Joystick (XY lock, pitch bend status, X+/X-/Y+/Y- assign and depth).
