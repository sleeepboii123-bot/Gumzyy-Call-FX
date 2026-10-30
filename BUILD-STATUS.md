# Build status — GUMZYY CALL FX v0.3.2

- [x] v0.3.2 telephone-led UI, six character chains, BIG/SMALL layouts, A/B, RANDOM and RESET.
- [x] Low-latency sample-by-sample effects and zero-sample plugin latency declaration.
- [x] v0.3.2 MSVC fixes: BinaryData resource filename uses a C++-safe underscore; mutable JUCE Rectangle is no longer const.
- [x] Local DSP tests passed with AddressSanitizer and UndefinedBehaviorSanitizer.
- [ ] Windows MSVC VST3 build passed for v0.3.2.
- [ ] Ableton Live UI, audio and automation verification.

The previous Windows build failed only in PluginEditor.cpp: the phone banner's generated BinaryData identifier did not match its hyphenated filename, and drawModule attempted to mutate a const Rectangle. Both are fixed in this patch. Do not mark this build final until GitHub Actions and Ableton verification pass.


## Linker fix

The v0.3.1 build linked unsuccessfully with LNK2001 for `GumzyyLookAndFeel::getTextButtonFont` and `GumzyyLookAndFeel::drawButtonText`. v0.3.2 supplies both member definitions. This is a UI-only link fix; the DSP implementation and zero-sample plugin latency declaration are unchanged.
