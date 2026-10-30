# Build status — GUMZYY CALL FX v0.2.1 WORKFLOW

- [x] Retained the sample-by-sample DSP path from v0.1.0.
- [x] Added Input Gain and Tone as automatable DSP parameters.
- [x] Reworked the UI into a workflow-first layout with six quick character buttons.
- [x] Added real input/output peak meters sourced from the audio processor.
- [x] Added tooltips and double-click knob reset.
- [x] Added controlled random variation per ad-lib character.
- [x] Added A/B snapshots for comparing two full effect settings.
- [x] Added reset-to-character and compact/full UI switching.
- [x] Removed non-ASCII glyphs from interface labels to avoid Windows font/codepage artefacts.
- [ ] Windows MSVC build and VST3 packaging.
- [ ] Verify all parameters respond audibly in Ableton Live.
- [ ] Check A/B, randomize, reset, automation, and UI resize in Ableton.
- [ ] Listening tests for ANGEL, PHONE, RADIO, WALKIE, MEGA and BROKEN.

The engine keeps its zero-lookahead, sample-by-sample design and preallocated delay/reverb buffers. This is a zero-sample declared plugin latency path, not a claim of zero total recording latency. CPU and sound-quality checks still require a DAW session.


## v0.2.1 build fix

- Fixed the MSVC compile error C2371 in PluginEditor.cpp by giving the footer control layout its own variable name (`footerControlsY`). The panel layout retains `footerY`.
