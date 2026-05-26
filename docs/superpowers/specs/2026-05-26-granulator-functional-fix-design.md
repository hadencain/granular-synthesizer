# Granulator — Functional Fix Pass

**Date:** 2026-05-26  
**Scope:** Functional completeness only. UI polish is a separate pass.  
**Status:** Approved

---

## Problem Summary

The granulator plugin has several functional gaps:

1. Five ComboBoxes are attached to APVTS parameters but never populated with items — they appear blank and do nothing when clicked.
2. The RecordSection's REC and OPEN FOLDER buttons are clipped by insufficient height allocation in the PluginEditor layout.
3. `grain_probability` exists in the APVTS and DSP but has no UI control.
4. Envelope Modulators (2× ADSR, 8 params) and Envelope Follower (attack + release, 2 params) exist in the APVTS and DSP but have no UI at all.

Parameters `dc_filter`, `anti_aliasing`, and their current hidden state are intentional — leave them as internal defaults with no UI.

---

## Fix 1 — Populate Empty ComboBoxes

**Files:** `PitchSection.cpp`, `FilterSection.cpp`, `BufferSection.cpp`, `EffectsSection.cpp`

Each of these ComboBoxes is declared, attached to its APVTS parameter via `ComboBoxAttachment`, but never had `addItem()` or `addItemList()` called. Population must happen **before** the attachment is created so the attachment can set the initial value correctly.

| Section | Member | Parameter ID | Items |
|---|---|---|---|
| PitchSection | `transposeBox` | `transpose_mode` | Doppler, Interpolated, Phase Vocoder |
| PitchSection | `quantizeBox` | `pitch_quantize` | Off, Chromatic, Major, Minor, Custom |
| FilterSection | `filterTypeBox` | `filter_type` | Low-Pass, High-Pass, Band-Pass, Notch |
| BufferSection | `recordModeBox` | `record_mode` | Off, Continuous, One-Shot, Gate |
| EffectsSection | `wsTypeBox` | `fx_ws_type` | Tanh, Soft Clip |

Item IDs must be 1-based integers matching APVTS choice indices (JUCE ComboBox convention). Use `addItemList()` with a `StringArray` for clarity.

---

## Fix 2 — Record Button Clipping

**Files:** `PluginEditor.cpp`, optionally `RecordSection.cpp`

**Root cause:** RecordSection is allocated 25% of the column height (~151px at the 604px body height). Its internal layout places the REC and OPEN FOLDER buttons at `y=136`, leaving only ~15px below — the buttons are cut off.

**Fix:** Increase RecordSection's proportional height in `PluginEditor::resized()` from 25% to ~35% (≈211px). Reduce EffectsSection from 75% to 65% proportionally. At 211px, the buttons at y=136 have ~75px clearance.

Alternative if shrinking EffectsSection is undesirable: compact RecordSection's internal layout — reduce the time label font from 22pt to 16pt and tighten header padding to recover ~30px, fitting within the current 151px bound.

**Preferred:** Height reallocation (less risky than reflowing RecordSection internals).

---

## Fix 3 — grain_probability Knob in PlayheadSection

**Files:** `PlayheadSection.h`, `PlayheadSection.cpp`

Add a `KnobWithLabel` for `grain_probability` (range 0–1, label "PROB") to PlayheadSection.

- Declare member: `KnobWithLabel probKnob;`
- Create `SliderAttachment`: `probAttachment` bound to `grain_probability`
- Layout: PlayheadSection currently uses a 2×3 knob grid. Extend to 2×4 (add a third row), placing PROB in the first slot of row 3 alongside FREEZE toggle or independently.
- `addAndMakeVisible(probKnob)` in constructor; `probKnob.setBounds(...)` in `resized()`

---

## Fix 4 — Envelope Modulators + Follower in ModSection

**Files:** `ModSection.h`, `ModSection.cpp`

Stack three new control strips below the existing 4 LFO strips, above the ModMatrixGrid. All use the existing `KnobWithLabel` component.

### Env Mod Strips (×2)

Each strip mirrors the LFO strip structure but with 4 knobs instead of 3+combo:

| Knob | Parameter ID |
|---|---|
| Attack | `envmod1_attack_ms` / `envmod2_attack_ms` |
| Decay | `envmod1_decay_ms` / `envmod2_decay_ms` |
| Sustain | `envmod1_sustain` / `envmod2_sustain` |
| Release | `envmod1_release_ms` / `envmod2_release_ms` |

Strip header labels: `ENV 1`, `ENV 2`.

### Env Follower Strip

Two knobs:

| Knob | Parameter ID |
|---|---|
| Attack | `ef_attack_ms` |
| Release | `ef_release_ms` |

Strip header label: `FOLLOWER`.

### Layout

ModSection is in column 5 (280px wide). The three new strips each use the same height as an LFO strip. Total ModSection height increases by `3 × stripHeight`. Two options:

- **Preferred:** Make the ModMatrixGrid scrollable (set `setScrollBarsShown(true, false)`) and keep the plugin window at 1440×720. The grid already has 8 fixed rows — a scroll bar is acceptable since it's not a primary editing surface.
- **Fallback:** Increase plugin window height from 720 to ~820px in `PluginEditor` constructor and `getHeight()`.

Add `SliderAttachment` members for all 10 new params alongside the existing LFO attachments.

### Section Labels

Add a visual divider row between LFOs and ENV MODS, and between ENV MODS and FOLLOWER — matching whatever separator style currently exists between ModSection sub-areas.

---

## Out of Scope

- UI polish / aesthetic redesign (separate pass)
- `dc_filter` and `anti_aliasing` — remain internal defaults, no UI
- Any DSP changes — all parameters already function correctly in the engine

---

## Files Touched

| File | Change |
|---|---|
| `source/ui/sections/PitchSection.cpp` | Populate `transposeBox`, `quantizeBox` |
| `source/ui/sections/FilterSection.cpp` | Populate `filterTypeBox` |
| `source/ui/sections/BufferSection.cpp` | Populate `recordModeBox` |
| `source/ui/sections/EffectsSection.cpp` | Populate `wsTypeBox` |
| `source/ui/sections/PlayheadSection.h` | Add `probKnob`, `probAttachment` |
| `source/ui/sections/PlayheadSection.cpp` | Wire + lay out `probKnob` |
| `source/ui/sections/ModSection.h` | Add env mod + follower strips and attachments |
| `source/ui/sections/ModSection.cpp` | Wire + lay out new strips; scrollable grid |
| `source/PluginEditor.cpp` | Increase RecordSection height allocation |
