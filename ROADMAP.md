# Roadmap: Cobalt8M controller

Controller for the Modal Cobalt8M, in the style of Modal's own app.

## Next
- [ ] Real MIDI I/O behind the pages
- [ ] Pass-through for DAW MIDI on by default with an automatic loop guard, switches in Settings
- [ ] Joystick circle fix in the mockup
- [ ] Later: other Modal synths (for example Argon8) for other users

## Every cat plugin
- Draw through `catui::RenderGate` so meters and animations stop when the window is not in front (audio, automation and MIDI keep running)
- Settings page with MIDI filters on by default, selectable preset/backup folders, UI size reset to 100%
