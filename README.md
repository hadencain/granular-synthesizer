# Granulator

A granular synthesis VST3 plugin built with JUCE 7.

## Features

- **Grain engine** — up to 512 simultaneous grains per voice, 16-voice polyphony
- **Playhead control** — position, spray, scan rate/shape (Forward, Reverse, Pendulum, Random Walk, Sine), loop start/end, freeze, scrub, grain probability
- **Pitch** — pitch shift, detune, randomization, playback rate, transpose mode (Doppler / Interpolated / Phase Vocoder), pitch quantize, formant shift, glide
- **Amplitude** — amplitude, ADSR envelope, velocity sensitivity, crossfade
- **Spatial** — pan, pan randomization, stereo width, voice count with detuned unison
- **Filter** — state-variable filter (Low-Pass, High-Pass, Band-Pass, Notch) with envelope and LFO depth, keytracking
- **Buffer** — live input recording with four modes (Off, Continuous, One-Shot, Gate), feedback, input gain, file load
- **Effects** — waveshaper (Tanh / Soft Clip), 4-band parametric EQ, chorus, delay (with ping-pong), reverb, limiter
- **Modulation** — 4 LFOs (5 shapes), 2 ADSR envelope modulators, envelope follower, 8-slot mod matrix routing any source to any destination
- **Waveform display** — interactive: click to set playhead position, drag to define loop region, real-time grain position markers

## Requirements

- CMake 3.22+
- C++17 compiler (MSVC on Windows, Clang on macOS)
- Ninja build system
- JUCE is fetched automatically via CPM at configure time — no manual installation needed

## Build

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The compiled VST3 is output to `build/Granulator_artefacts/Release/VST3/Granulator.vst3`.

Copy it to your DAW's VST3 folder:
- **Windows:** `C:\Program Files\Common Files\VST3\`
- **macOS:** `~/Library/Audio/Plug-Ins/VST3/`

## Project Structure

```
source/
├── PluginProcessor.cpp/h     — AudioProcessor, APVTS, audio thread
├── PluginEditor.cpp/h        — AudioProcessorEditor, top-level UI layout
├── grain/                    — Grain engine (GranularEngine, GrainBuffer, Grain, GrainEnvelope)
├── synth/                    — JUCE Synthesiser wrapper (GranularSynth, GranularVoice)
├── dsp/                      — Effects chain, GranularParams snapshot struct
├── modulation/               — LFO, EnvelopeFollower, ModMatrix
└── ui/
    ├── GranularLookAndFeel   — Custom JUCE LookAndFeel (dark theme)
    ├── components/           — KnobWithLabel, WaveformDisplay, ModMatrixGrid
    └── sections/             — One component per UI panel column
```
