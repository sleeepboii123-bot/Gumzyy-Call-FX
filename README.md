# GUMZYY CALL FX v0.2.1 - WORKFLOW UPDATE

**NYXV × OFF SESSION — AD-LIB COLOR LAB**

A Windows VST3 effect concept for giving ad-libs and vocal doubles a recognisable character: angelic/airy, telephone, radio, walkie-talkie, megaphone and broken-digital.

## Low-latency design target

- The plugin reports **0 samples of host plugin latency** (`setLatencySamples(0)`).
- No FFT pitch shifting, look-ahead, oversampling, convolution partitioning or block-based effect buffering is used.
- Filters, waveshaping, stereo width and bit/sample reduction run sample-by-sample.
- Delay and reverb use preallocated circular buffers on the wet path. The direct/dry path is not delayed; the echo itself is an intentional effect, not plugin compensation latency.
- Scratch buffers and delay/reverb memory are allocated in `prepareToPlay`, not inside `processBlock`.
- Parameter changes are smoothed to reduce zipper noise and clicks.

**Important:** 0 samples of reported plugin latency is an architectural property, not a measured CPU benchmark. Real-time CPU load, stability and audible quality still need validation in Ableton Live on the target computer. High drive/crush settings can create aliasing; this first prototype deliberately avoids oversampling to preserve the low-latency design.

## Main styles

- **ANGEL** — airy high-frequency lift, wide stereo, reverb and delay.
- **PHONE** — narrow telephone band-pass and mild saturation.
- **RADIO** — tighter bandwidth, more drive and a touch of lo-fi texture.
- **WALKIE** — narrow, aggressive band and stronger crush.
- **MEGAPHONE** — forward mid-focused voice with saturation.
- **BROKEN** — stronger bit/sample reduction and glitchy echo character.

The style selector applies a starting preset by writing the exposed APVTS parameters. All knobs remain host-automatable in the DAW.

## Controls and ad-lib workflow

- **FILTER / VOICE:** High Pass, Low Pass, Input Gain and Tone.
- **ANGEL / STEREO:** Air and Width.
- **COLOR / GRIT:** Drive and Crush.
- **SPACE / ECHO:** Reverb, Delay, Delay Time and Feedback.
- **Output strip:** live input/output peak meters, Mix and Output.
- **Workflow:** six one-click character chains, A/B snapshots, bounded Random, Reset-to-character, Bypass and BIG/SMALL view.

Every knob is attached to a real APVTS parameter, so it changes the DSP and is available for Ableton automation. Double-click a knob to reset it. A/B stores the current setup as A, captures your edited setup as B, then toggles between them. Random creates a controlled variation around the selected character rather than randomising every control across its full range.

GUMZYY artwork is embedded in the VST3 binary, so there are no external image dependencies at install time. See `PRESET-GUIDE.md` for ad-lib starting points.

## Build on Windows

Requirements: Git for Windows, GitHub CLI (`gh`) authenticated to GitHub, and a network connection for CMake to fetch JUCE 8.0.6.

1. Extract this project ZIP into a new folder.
2. Run `Publish-To-GitHub.bat`.
3. The helper creates a new public repository named `sleeepboii123-bot/Gumzyy-Call-FX` if it does not exist, then pushes the project. It does not modify the former OFF-SESSION-TUNE repository.
4. Open the repository's Actions tab and wait for the Windows x64 build and DSP tests.
5. Download `GUMZYY-CALL-FX-v0.2.1-Windows-x64-VST3` from the successful run.
6. Extract the artifact and run `Install-GUMZYY-CALL-FX.bat`.

## Licensing note

JUCE 8.0.6 is dual-licensed under AGPLv3 or a commercial JUCE licence. Review the JUCE 8 EULA before distributing a closed-source build or selling the plugin. See `THIRD-PARTY-NOTICES.md`.

## Project status

This is **v0.2.1 — workflow update source candidate**. The DSP engine retains the v0.1.0 sample-by-sample path. Local DSP tests cover zero reported latency, dry-path integrity, phone filtering, crush stability, stereo processing and common sample rates. The updated JUCE editor still requires a Windows MSVC build, installation and listening/automation verification in Ableton Live.

The Windows installer now requests administrator permission automatically, waits for the elevated install to finish and returns its exit status.
