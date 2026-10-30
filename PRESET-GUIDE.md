# Starting points for ad-libs

These are starting presets, not mastering targets. Match output level before comparing.

## ANGEL

- Shimmer: 45-70% for the high-band harmonic glow (not an octave pitch shifter)

- Air: 6–9 dB
- Width: 125–160%
- Reverb: 35–60%
- Delay: 15–30%
- Keep Drive low unless the vocal needs more density.

Designed for airy layers and wide ad-libs. It does not pitch-shift the vocal upward; the angelic quality comes from air, width and space in this first version.

## PHONE

- High Pass: around 250–400 Hz
- Low Pass: around 2.8–4.0 kHz
- Drive: 10–25%
- Reverb/Delay: near zero for the classic narrow call sound.

## RADIO

- High Pass: around 120–250 Hz
- Low Pass: around 4.5–7 kHz
- Drive: 25–40%
- Crush: 5–20%

## WALKIE

- High Pass: around 400–650 Hz
- Low Pass: around 2.2–3.2 kHz
- Drive: 35–55%
- Crush: 20–45%

## MEGAPHONE

- High Pass: around 180–300 Hz
- Low Pass: around 3.5–5 kHz
- Drive: 20–35%

## BROKEN

- Raise Crush gradually and add Delay sparingly. This mode is intended to sound deliberately degraded.

## Recording tips

- Put the effect on an ad-lib track or send, not automatically on the lead vocal.
- Keep Mix at 100% when inserting on an ad-lib track; use a send/return for parallel use.
- Check Width in mono. Very wide settings can weaken the centre when summed.
- Keep the DAW's input peak healthy; do not clip the interface before the plugin.


## Workflow controls in v0.3.1

- Character buttons load a useful starting chain. Adjust the knobs after loading; the buttons are starting points, not locked algorithms.
- RANDOM generates a bounded variation around the selected character.
- RESET returns the selected character to its starting values.
- A/B stores the current setup as A, captures your edited setup as B, then switches between them. After changing either side, press A/B again to update that side before switching.
- Double-click any knob to restore its default value.
- Input and output meters show live block peak levels. They are peak indicators, not LUFS meters.
- Input Gain changes the level before tone and saturation. Tone is a broad low/high tilt. Both are exposed as host-automatable DSP parameters.
