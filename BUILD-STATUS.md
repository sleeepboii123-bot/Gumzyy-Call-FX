# Build status — GUMZYY CALL FX v0.3.1

- [x] v0.3.1 telephone-led UI, six character chains, BIG/SMALL layouts, A/B, RANDOM and RESET.
- [x] Low-latency sample-by-sample effects and zero-sample plugin latency declaration.
- [x] v0.3.1 MSVC fixes: BinaryData resource filename uses a C++-safe underscore; mutable JUCE Rectangle is no longer const.
- [x] Local DSP tests passed with AddressSanitizer and UndefinedBehaviorSanitizer.
- [ ] Windows MSVC VST3 build passed for v0.3.1.
- [ ] Ableton Live UI, audio and automation verification.

The previous Windows build failed only in PluginEditor.cpp: the phone banner's generated BinaryData identifier did not match its hyphenated filename, and drawModule attempted to mutate a const Rectangle. Both are fixed in this patch. Do not mark this build final until GitHub Actions and Ableton verification pass.
