# Roadmap: Cobalt8M controller

Controller for the Modal Cobalt8M, in the style of Modal's own app.

## Done
- [x] Plugin skeleton: parameter map from the manual, CC and NRPN send/receive, host automation, cat-ui look, CI on three systems

## Next
- [ ] Test every map entry on the real device and flip it to verified (`hw` column in map/params.csv)
- [ ] Test the VST3 in Bitwig and Studio One on Linux
- [ ] Two-CC controls: find out which CC is the coarse one on the device

- [ ] Real MIDI I/O behind the pages
- [ ] Pass-through for DAW MIDI on by default with an automatic loop guard, switches in Settings
- [ ] Joystick circle fix in the mockup
- [ ] Later: other Modal synths (for example Argon8) for other users

## Every cat plugin
- Draw through `catui::RenderGate` so meters and animations stop when the window is not in front (audio, automation and MIDI keep running)
- Settings page with MIDI filters on by default, selectable preset/backup folders, UI size reset to 100%

## Joystick Y+
The manual's CC chart has no entry for Joystick Y+. In our plugin the joystick is visual and springs back to center (a small easter egg). Optional later: a custom matrix of our own that maps the joystick to any parameter.
