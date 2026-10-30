# GUMZYY CALL FX v0.2.1 test plan

## Build and package

1. Publish the source to the existing `sleeepboii123-bot/Gumzyy-Call-FX` repository.
2. Confirm the Windows x64 Release workflow compiles the VST3 and DSP test executable.
3. Confirm CTest passes and the artifact contains the `.vst3` bundle plus both installer files.

## Ableton setup

1. Close Ableton Live before installing.
2. Extract the artifact and run `Install-GUMZYY-CALL-FX.bat`.
3. Approve the Windows UAC prompt. The elevated installer waits until the copy completes and returns the result.
4. Restart Ableton or rescan VST3 plug-ins.
5. Set the audio buffer to 64 or 128 samples for the first live-monitoring test. This is an audio-device setting, not a value forced by the plugin.

## Workflow checks

- Click ANGEL, PHONE, RADIO, WALKIE, MEGA and BROKEN. Verify each character loads its starting settings and updates the knobs.
- Move HP, LP, Input, Tone, Air, Width, Drive, Crush, Reverb, Delay, Time and Feedback. Verify each control changes the sound and remains available for host automation.
- Confirm live IN and OUT meters respond to audio and to bypassed audio.
- Test A/B: save A, change several knobs for B, press A/B to compare, then switch back and forth.
- Test RANDOM for bounded variations and RESET to restore the selected character.
- Test BIG/SMALL resizing and double-click reset on several knobs.

## Listening checks

- PHONE: narrow 250-400 Hz to 2.8-4 kHz as a starting point; adjust for the voice.
- RADIO: add moderate Drive and a small amount of Crush; loudness-match when comparing.
- ANGEL: add Air, Width and space; check the mono sum for loss of vocal level.
- WALKIE / MEGA: listen for harshness and excessive consonants.
- BROKEN: raise Crush carefully and verify the degradation is intentional.

## Recording safety and performance

- Record a dry ad-lib and a processed version at the same input gain.
- Listen for clicks, dropouts, crackles, unexpected pumping or CPU spikes.
- Test 44.1 kHz and 48 kHz, mono and stereo, and 64/128/256-sample device buffers.
- Verify the reported plugin latency remains 0 samples. The device buffer and interface still contribute their own latency.
