# Granulator UI Redesign — Design Spec
**Date:** 2026-05-25  
**Status:** Approved

---

## Overview

Full UI overhaul of the JUCE granulator VST3 plugin. Remove the 10-tab interface and replace with a single maximalist master view where every parameter is visible at once. Add grain probability parameter, preset browser, and logo-triggered randomize. Expand window from 900×620 to 1400×920.

---

## Window

- **Size:** 1400×920 px (fixed, no resize)
- **Background:** `#0c0c0c`
- Update `PluginEditor` constructor to call `setSize(1400, 920)`

---

## Layout Structure

```
┌──────────────────────────────────────────────────────────────────┐
│ HEADER (40px): [GRANULATOR logo] [PresetBar] [grain counter]     │
├────────────┬────────────┬────────────┬────────────┬─────────────┤
│  COL 1     │  COL 2     │  COL 3     │  COL 4     │  COL 5      │
│  ~265px    │  ~265px    │  ~265px    │  ~265px    │  ~260px     │
│            │            │            │            │             │
│  GRAIN     │  PITCH     │  SPACE     │  FX CHAIN  │  LFO        │
│  PLAYHEAD  │  AMPLITUDE │  FILTER    │            │  ROUTING    │
│            │            │  BUFFER    │            │  PANEL      │
│            │            │  (waveform)│            │             │
│            │            │  RECORD    │            │  OUTPUT     │
│            │            │            │            │  (VOL/WET)  │
└────────────┴────────────┴────────────┴────────────┴─────────────┘
```

**Padding:** 8px edges, 8px column gaps  
**Column heights:** full 880px body (920 - 40px header)  
**Column dividers:** 1px `#1e1e1e` vertical lines between columns

---

## Header (40px)

Left → right:
1. **GRANULATOR label** — white, 18px, letter-spacing 4px, bold. Cursor changes to `PointingHandCursor` on hover. `mouseDown` triggers `randomizeAllParameters()`. On click: label briefly pulses — color transitions from `#ffffff` → `#888888` → `#ffffff` over 200ms using a `juce::Timer`.
2. **PresetBar** — centered, see Preset Browser section below.
3. **Grain counter** — right-aligned, `#555555`, 10px. Text: `N GRAINS`.

Header background: `#0e0e0e`. Bottom border: 1px `#1e1e1e`.

---

## Column 1 — Grain + Playhead

### GRAIN section
Parameters (all existing except `grain_probability` is new):
- **SIZE** (`grain_size_ms`) — rotary knob
- **DENSITY** (`grain_density`) — rotary knob
- **OVERLAP** (`grain_overlap`) — rotary knob
- **INTRONSET** (`interonset_ms`) — rotary knob
- **RND SIZE** (`randomize_size_ms`) — rotary knob
- **RND DENS** (`randomize_density`) — rotary knob
- **PROB** (`grain_probability`) — rotary knob, **NEW**, 0–100%, default 100%
- **ENVELOPE** (`grain_envelope`) — ComboBox: Hann / Gaussian / Trapezoid / Rectangle / Triangular / Tukey
- **DIRECTION** (`grain_direction`) — ComboBox: Forward / Reverse / Bidirectional / Random

### PLAYHEAD section
- **POSITION** (`position`) — rotary knob
- **SPRAY** (`spray_ms`) — rotary knob
- **SCAN RATE** (`scan_rate_hz`) — rotary knob
- **LOOP ST** (`loop_start`) — rotary knob
- **LOOP END** (`loop_end`) — rotary knob
- **SCRUB** (`scrub`) — rotary knob
- **SCAN SHAPE** (`scan_shape`) — ComboBox: Forward / Reverse / Pendulum / Random Walk / Sine
- **FREEZE** (`freeze`) — ToggleButton

---

## Column 2 — Pitch + Amplitude

### PITCH section
- **SHIFT** (`pitch_shift_st`) — rotary knob
- **DETUNE** (`detune_cents`) — rotary knob
- **RND** (`pitch_random_st`) — rotary knob
- **RATE** (`playback_rate`) — rotary knob
- **FORMANT** (`formant_shift_st`) — rotary knob
- **GLIDE** (`glide_ms`) — rotary knob
- **TRANSPOSE** (`transpose_mode`) — ComboBox: Doppler / Interpolated / Phase Vocoder
- **QUANTIZE** (`pitch_quantize`) — ComboBox: Off / Chromatic / Major / Minor / Custom

### AMPLITUDE section
- **AMP** (`amplitude`) — rotary knob
- **RND** (`amp_random_db`) — rotary knob
- **VEL** (`velocity_sensitivity`) — rotary knob
- **XFADE** (`crossfade_ms`) — rotary knob
- **ATK** (`adsr_attack_ms`) — rotary knob
- **DEC** (`adsr_decay_ms`) — rotary knob
- **SUS** (`adsr_sustain`) — rotary knob
- **REL** (`adsr_release_ms`) — rotary knob

---

## Column 3 — Space + Filter + Buffer + Record

### SPACE section
- **PAN** (`pan`) — rotary knob
- **PAN RND** (`pan_random`) — rotary knob
- **WIDTH** (`stereo_width`) — rotary knob
- **VOICES** (`voices`) — rotary knob (integer 1–16)
- **V.DETUNE** (`voice_detune_cents`) — rotary knob

### FILTER section
- **TYPE** (`filter_type`) — ComboBox: Low-Pass / High-Pass / Band-Pass / Notch
- **CUTOFF** (`filter_cutoff_hz`) — rotary knob
- **RES** (`filter_resonance`) — rotary knob
- **ENV** (`filter_env_depth`) — rotary knob
- **LFO** (`filter_lfo_depth`) — rotary knob
- **KEYTRK** (`filter_keytrack`) — rotary knob

### BUFFER section
- **WaveformDisplay** — 140px tall, full column width, existing component
- **LENGTH** (`buffer_length_ms`) — rotary knob
- **MODE** (`record_mode`) — ComboBox: Off / Continuous / One-Shot / Gate
- **FEEDBACK** (`record_feedback`) — rotary knob
- **GAIN** (`input_gain_db`) — rotary knob
- **LOAD FILE** — TextButton
- **CLEAR** — TextButton

### RECORD section
- **REC** — TextButton (toggles recording to disk)
- **OPEN FOLDER** — TextButton
- Status label, time label, file label (read-only displays)

---

## Column 4 — FX Chain

All FX parameters displayed in a single vertical chain, each sub-section separated by a `#1e1e1e` hairline:

### Waveshaper
- **DRIVE** (`fx_drive`) — rotary knob
- **TYPE** (`fx_ws_type`) — ComboBox: Tanh / Soft Clip

### EQ (4 bands, displayed as a 4-column sub-grid)
Each band: **FREQ**, **GAIN**, **Q** knobs
- Band 1: `fx_eq1_freq`, `fx_eq1_gain`, `fx_eq1_q`
- Band 2: `fx_eq2_freq`, `fx_eq2_gain`, `fx_eq2_q`
- Band 3: `fx_eq3_freq`, `fx_eq3_gain`, `fx_eq3_q`
- Band 4: `fx_eq4_freq`, `fx_eq4_gain`, `fx_eq4_q`

### Chorus
- **RATE** (`fx_chorus_rate`), **DEPTH** (`fx_chorus_depth`), **MIX** (`fx_chorus_mix`) — rotary knobs

### Delay
- **TIME** (`fx_delay_time_ms`), **FDBK** (`fx_delay_feedback`), **MIX** (`fx_delay_mix`) — rotary knobs
- **PING PONG** (`fx_delay_pingpong`) — ToggleButton

### Reverb
- **ROOM** (`fx_reverb_room`), **DAMP** (`fx_reverb_damp`), **MIX** (`fx_reverb_mix`) — rotary knobs

### Limiter
- **THRESH** (`fx_limiter_thresh`), **RELEASE** (`fx_limiter_release`) — rotary knobs

---

## Column 5 — LFO Routing Panel

Background: `#0a0a0a`. Left border: 1px `#1e1e1e`.

### 4 LFO Strips (stacked vertically, equal height)

Each strip contains:
- **Title label:** `LFO N — SHAPE RATEHz` (e.g. `LFO 1 — SINE 1.0Hz`) — white, 9px, letter-spacing 1px. Updates live as shape/rate change.
- **Knobs row:** RATE, DEPTH, PHASE (small, ~18px diameter)
- **Shape dropdown:** ComboBox with 7 shapes
- **Routing list:** reads active mod matrix slots where source == LFO N. Each active slot renders as one line:
  ```
  ● → CUTOFF   ×0.40
  ● → PAN      ×0.20
  ```
  Dot color: white. Target name: `#aaaaaa`. Depth value: `#666666`. Right-aligned.
  If no routings active: `— unpatched —` in `#444444`.
- Strip separator: 1px `#1e1e1e` horizontal line

The routing list is **read-only display** — it reflects live mod matrix state. The full mod matrix editor is removed from the UI (it was only in the MOD tab which no longer exists). Modulation routing is now managed by right-clicking a knob (future work, not in this build). For this build: existing mod matrix state still applies at the DSP level, routing display shows current state.

> **Note:** The existing `ModSection` and `ModMatrixGrid` components are removed from the UI. The mod matrix DSP (`ModMatrix.h/cpp`) is unchanged — it still processes all routings. The LFO Routing Panel is a new read-only view over the same data.

### Output (bottom of Col 5)
- **VOL** (`master_volume_db`) — rotary knob
- **WET** (`dry_wet`) — rotary knob
- **DC** (`dc_filter`) — ToggleButton (small)
- **AA** (`anti_aliasing`) — ComboBox (small): Off / Low / High

---

## New Feature: Grain Probability

**Parameter:** `grain_probability`  
**Type:** float, 0.0–1.0, default 1.0  
**Display:** 0–100%  
**APVTS ID:** `"grain_probability"`

**DSP change** (`GranularEngine.cpp`): In the grain scheduling loop, before calling `spawnGrain()`, add:

```cpp
float prob = *rawParams.grainProbability;
if (prob < 1.0f && random.nextFloat() > prob)
    continue; // skip this grain
```

**UI placement:** GRAIN section, Col 1. Knob labeled `PROB`.

**Randomize behavior:** included in full randomize (random value 0.2–1.0, not 0.0–1.0, to avoid fully muting the instrument).

---

## New Feature: Preset Browser

### PresetManager (new class: `source/ui/PresetManager.h/cpp`)

Responsibilities:
- **Storage location:** `juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Granulator/Presets/")`
- **Format:** `.granpreset` files — XML serialization of `processor.apvts.state`
- **API:**
  ```cpp
  void savePreset(const juce::String& name);
  void loadPreset(const juce::File& file);
  juce::Array<juce::File> getPresetList();
  juce::String getCurrentPresetName();
  void setCurrentPresetName(const juce::String& name);
  ```
- On `savePreset`: serialize `apvts.state` to XML, write to `<name>.granpreset`, update current name
- On `loadPreset`: read XML, restore to `apvts.state`, update current name
- Factory presets: none for v1. Folder created on first save if it doesn't exist.

### PresetBar (new component: `source/ui/components/PresetBar.h/cpp`)

Lives in the header, centered between the logo and grain counter. Width: ~500px.

Layout left→right:
```
[<] [PRESET NAME ▼] [>] [SAVE]
```

- **`<` / `>`** — TextButtons, step through preset list in alphabetical order. Wrap at ends.
- **Preset name** — Label showing current preset name. Default text: `— init —`. Click opens `PresetBrowserPopup`.
- **`▼`** — small dropdown arrow, also opens `PresetBrowserPopup`
- **SAVE** — TextButton. If current preset has a name (was loaded from file), overwrites it. If `— init —`, opens a `juce::AlertWindow` with a text input for the preset name, then saves.

### PresetBrowserPopup

A `juce::Component` shown as a `juce::CalloutBox` anchored to the preset name label. Contains:
- A `juce::ListBox` of all `.granpreset` files in the presets folder, sorted alphabetically
- Each row: preset name (filename without extension)
- Click a row: load preset, close popup
- A **DELETE** button below the list: deletes the selected preset file (with confirmation via `juce::AlertWindow`)
- Popup closes on click-outside via `juce::ModalComponentManager` or `mouseDownSelectsItems`

### Randomize + Preset interaction

After randomize: preset name resets to `— init —` (state is now unsaved). User can save the result as a new preset via SAVE.

---

## New Feature: Randomize All Parameters

**Trigger:** `mouseDown` on the GRANULATOR header label.

**Implementation** (`PluginEditor.cpp`):

```cpp
void randomizeAllParameters()
{
    juce::Random rng;
    auto& apvts = processor.apvts;

    // Parameters excluded from randomize:
    static const juce::StringArray excluded = {
        "record_mode", "dc_filter", "anti_aliasing"
    };

    for (auto* param : apvts.processor.getParameters())
    {
        auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(param);
        if (rangedParam == nullptr) continue;
        if (excluded.contains(rangedParam->paramID)) continue;

        // grain_probability: clamp min to 0.2 (avoid silent instrument)
        float normVal;
        if (rangedParam->paramID == "grain_probability")
            normVal = juce::jmap(rng.nextFloat(), 0.2f, 1.0f);
        else
            normVal = rng.nextFloat();

        rangedParam->setValueNotifyingHost(normVal);
    }

    // Reset preset name to init
    presetManager.setCurrentPresetName("— init —");
    presetBar.updateName();
}
```

**Visual feedback:** On click, the GRANULATOR label color animates: white → `#555555` → white over 200ms. Implemented with a `juce::Timer` firing at 30fps for 200ms, interpolating alpha.

---

## Contrast & Typography Fixes

All label text must pass WCAG AA (4.5:1 contrast on dark backgrounds).

| Element | Old color | New color |
|---------|-----------|-----------|
| Section header labels | `#909090` | `#ffffff` |
| Knob parameter labels | `#909090` | `#cccccc` |
| Section divider labels | `#909090` | `#aaaaaa` |
| Inactive tab text | `#909090` | removed (no tabs) |
| Combo box text | `#d0d0d0` | `#d0d0d0` (unchanged) |
| Routing list target names | — | `#aaaaaa` |
| Routing list depth values | — | `#777777` |
| Routing list empty state | — | `#444444` |
| LFO strip title | — | `#ffffff` |

No `withAlpha()` calls below `0.6f` on any text-rendering color.

---

## Components Added / Modified / Removed

| Component | Action | Notes |
|-----------|--------|-------|
| `PluginEditor` | Modified | Remove TabbedComponent, add MasterView layout |
| `GranularLookAndFeel` | Modified | Contrast fixes, larger window constants |
| `GrainCoreSection` | Modified | Add `grain_probability` knob |
| `ModSection` | Removed from UI | DSP layer unchanged |
| `ModMatrixGrid` | Removed from UI | DSP layer unchanged |
| `LFORoutingPanel` | **New** | Read-only LFO + routing display (Col 5) |
| `PresetBar` | **New** | Header preset nav controls |
| `PresetBrowserPopup` | **New** | Popup preset list |
| `PresetManager` | **New** | File I/O for `.granpreset` files |
| All other sections | Kept, embedded flat | Strip tab wrapping, embed directly in MasterView |

---

## APVTS Changes

One new parameter added to the parameter layout in `PluginProcessor::createParameterLayout()`:

```cpp
layout.add(std::make_unique<juce::AudioParameterFloat>(
    "grain_probability", "Grain Probability",
    juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
    1.0f
));
```

Add raw pointer to `RawParams` struct:
```cpp
std::atomic<float>* grainProbability { nullptr };
```

Wire in `PluginProcessor::cacheRawParameters()`:
```cpp
raw.grainProbability = apvts.getRawParameterValue("grain_probability");
```

---

## Files Changed Summary

```
source/
├── PluginEditor.h/.cpp              — full rewrite of layout, add randomize handler
├── PluginProcessor.h/.cpp           — add grain_probability param + cache ptr
├── grain/GranularEngine.cpp         — add probability gate in grain scheduling
├── ui/GranularLookAndFeel.h/.cpp    — contrast fixes, window size constants
├── ui/components/KnobWithLabel.h    — no change
├── ui/components/WaveformDisplay.h  — no change
├── ui/components/ModMatrixGrid.h    — kept (DSP), removed from editor paint cycle
├── ui/components/PresetBar.h/.cpp   — NEW
├── ui/components/PresetBrowserPopup.h/.cpp — NEW
├── ui/PresetManager.h/.cpp          — NEW
├── ui/LFORoutingPanel.h/.cpp        — NEW
└── ui/sections/
    ├── GrainCoreSection.h/.cpp      — add PROB knob
    ├── ModSection.h/.cpp            — kept (DSP refs), removed from editor
    └── (all others unchanged structurally)
```
