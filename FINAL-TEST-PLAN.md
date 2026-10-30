# First test plan

## Build

1. Build the VST3 with the Windows GitHub Actions workflow.
2. Confirm `GUMZYY_CALL_FX_DSP_TESTS` passes under CTest.
3. Confirm the artifact contains `GUMZYY CALL FX.vst3` and the installer.

## Ableton setup

1. Keep OFF SESSION TUNE separate; GUMZYY CALL FX has its own repository, bundle ID and plugin code.
2. Install the VST3 and rescan in Ableton Live.
3. Set the audio buffer to 64 or 128 samples for a first low-latency monitoring check. This is a DAW audio-device setting, not a value forced by this plugin.
4. Insert GUMZYY CALL FX on a mono or stereo vocal/ad-lib track.
5. Confirm the plugin reports 0 samples of plugin latency in the host's plugin/device information where available.

## Listening order

- PHONE: start with the preset; adjust HP and LP to taste.
- RADIO: compare Drive at 20%, 35% and 50%; match output loudness when comparing.
- ANGEL: raise Air modestly, then add Width and Reverb; check mono compatibility.
- WALKIE / MEGAPHONE: check harshness and sibilants.
- BROKEN: raise Crush gradually; verify the sound is intentionally degraded, not an unintended crackle.

## Recording safety

- Compare bypassed and enabled output at matched loudness.
- Record a short dry ad-lib and a processed version.
- Listen for clicks, dropouts, crackles, excessive pumping or CPU spikes.
- Test automation of style and each knob.
- Test 44.1 kHz and 48 kHz, mono and stereo, and 64/128/256 sample DAW buffers.
