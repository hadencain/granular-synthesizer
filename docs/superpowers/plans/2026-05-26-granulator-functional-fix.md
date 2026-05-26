# Granulator Functional Fix Pass — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix all non-functional UI controls — populate empty ComboBoxes, expose grain_probability, fix record button clipping, and add Envelope Mod + Follower strips to ModSection.

**Architecture:** Surgical edits to existing section files. No new files created. ComboBox population requires converting three non-pointer `ComboBoxAttachment` members to `unique_ptr` so items can be added before attachment creation. ModSection gains a `juce::Viewport` wrapping a content component that holds all strips (LFOs + EnvMods + Follower + MatrixGrid), enabling vertical scroll without resizing the plugin window.

**Tech Stack:** JUCE 7.0.12, C++17, CMake/Ninja, VST3 Standalone target

---

> **No automated test framework.** Each task's verification step is: build the Standalone target and confirm the UI change visually. Build command (from project root):
> ```bash
> cmake --build build --config Release --target Granulator_Standalone
> ```
> The standalone executable lands at `build/Granulator_artefacts/Release/Standalone/Granulator.exe`.

---

## File Map

| File | Change |
|---|---|
| `source/ui/sections/PitchSection.h` | `transposeAtt`, `quantizeAtt` → `unique_ptr` |
| `source/ui/sections/PitchSection.cpp` | Remove from init-list, add items, create attachments in body |
| `source/ui/sections/FilterSection.h` | `typeAtt` → `unique_ptr` |
| `source/ui/sections/FilterSection.cpp` | Remove from init-list, add items, create attachment in body |
| `source/ui/sections/BufferSection.h` | `recModeAtt` → `unique_ptr` |
| `source/ui/sections/BufferSection.cpp` | Remove from init-list, add items, create attachment in body |
| `source/ui/sections/EffectsSection.cpp` | Add `addItemList` before existing `wsTypeAtt` assignment |
| `source/ui/sections/PlayheadSection.h` | Add `prob` knob + `probAtt` |
| `source/ui/sections/PlayheadSection.cpp` | Wire + lay out `prob` in row-2 space |
| `source/ui/sections/ModSection.h` | Add `EnvModStrip`, follower knobs, `Viewport`, content component |
| `source/ui/sections/ModSection.cpp` | Implement EnvModStrip, wire follower, build scrollable layout |
| `source/PluginEditor.cpp` | Change `fxH` proportion from 0.75 → 0.65 |

---

## Task 1: Populate PitchSection ComboBoxes

**Files:**
- Modify: `source/ui/sections/PitchSection.h`
- Modify: `source/ui/sections/PitchSection.cpp`

**Context:** `transposeAtt` and `quantizeAtt` are declared as value-type `ComboBoxAttachment` members, which means they're constructed in the initializer list — before the constructor body runs and before any items can be added. JUCE's `ComboBoxAttachment` reads the current APVTS value to set the initial selection; if the ComboBox has no items at that moment, the initial value is lost. Fix: convert both to `unique_ptr`, move construction into the body after `addItemList`.

- [ ] **Step 1: Update PitchSection.h**

Replace the `ComboBoxAttachment` declarations (line 19):

```cpp
// Before:
juce::AudioProcessorValueTreeState::ComboBoxAttachment transposeAtt, quantizeAtt;

// After:
std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> transposeAtt, quantizeAtt;
```

- [ ] **Step 2: Update PitchSection.cpp initializer list**

Remove `transposeAtt` and `quantizeAtt` from the initializer list. The list becomes:

```cpp
PitchSection::PitchSection(juce::AudioProcessorValueTreeState& apvts)
    : pitchAtt  (apvts, "pitch_shift_st",  pitchShift.getSlider()),
      detuneAtt (apvts, "detune_cents",     detune.getSlider()),
      randAtt   (apvts, "pitch_random_st", pitchRandom.getSlider()),
      rateAtt   (apvts, "playback_rate",   playbackRate.getSlider()),
      formantAtt(apvts, "formant_shift_st",formantShift.getSlider()),
      glideAtt  (apvts, "glide_ms",        glide.getSlider())
{
```

- [ ] **Step 3: Add items and create attachments in constructor body**

After the label setup block and before the `addAndMakeVisible` calls, insert:

```cpp
    transposeBox.addItemList({"Doppler", "Interpolated", "Phase Vocoder"}, 1);
    quantizeBox.addItemList({"Off", "Chromatic", "Major", "Minor", "Custom"}, 1);
    transposeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "transpose_mode", transposeBox);
    quantizeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "pitch_quantize", quantizeBox);
```

- [ ] **Step 4: Build and verify**

```bash
cmake --build build --config Release --target Granulator_Standalone
```

Launch standalone. Open the Pitch section. Both "Transpose" and "Quantize" dropdowns should show their choices. Selecting one should not crash.

- [ ] **Step 5: Commit**

```bash
git add source/ui/sections/PitchSection.h source/ui/sections/PitchSection.cpp
git commit -m "fix: populate Pitch section transpose and quantize ComboBoxes"
```

---

## Task 2: Populate FilterSection ComboBox

**Files:**
- Modify: `source/ui/sections/FilterSection.h`
- Modify: `source/ui/sections/FilterSection.cpp`

Same issue as Task 1 — `typeAtt` is a value-type member initialized before items can be added.

- [ ] **Step 1: Update FilterSection.h**

Replace line 19:

```cpp
// Before:
juce::AudioProcessorValueTreeState::ComboBoxAttachment typeAtt;

// After:
std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAtt;
```

- [ ] **Step 2: Update FilterSection.cpp initializer list**

Remove `typeAtt` from the initializer list:

```cpp
FilterSection::FilterSection(juce::AudioProcessorValueTreeState& apvts)
    : cutoffAtt  (apvts, "filter_cutoff_hz", cutoff.getSlider()),
      resAtt     (apvts, "filter_resonance",  resonance.getSlider()),
      envDepthAtt(apvts, "filter_env_depth",  envDepth.getSlider()),
      lfoDepthAtt(apvts, "filter_lfo_depth",  lfoDepth.getSlider()),
      keytrackAtt(apvts, "filter_keytrack",   keytrack.getSlider())
{
```

- [ ] **Step 3: Add items and create attachment in constructor body**

After `filterTypeLabel.setText(...)` and before `addAndMakeVisible` calls:

```cpp
    filterTypeBox.addItemList({"Low-Pass", "High-Pass", "Band-Pass", "Notch"}, 1);
    typeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "filter_type", filterTypeBox);
```

- [ ] **Step 4: Build and verify**

```bash
cmake --build build --config Release --target Granulator_Standalone
```

Filter "Type" dropdown should show Low-Pass / High-Pass / Band-Pass / Notch.

- [ ] **Step 5: Commit**

```bash
git add source/ui/sections/FilterSection.h source/ui/sections/FilterSection.cpp
git commit -m "fix: populate Filter section type ComboBox"
```

---

## Task 3: Populate BufferSection ComboBox

**Files:**
- Modify: `source/ui/sections/BufferSection.h`
- Modify: `source/ui/sections/BufferSection.cpp`

- [ ] **Step 1: Update BufferSection.h**

Replace line 23:

```cpp
// Before:
juce::AudioProcessorValueTreeState::ComboBoxAttachment recModeAtt;

// After:
std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> recModeAtt;
```

- [ ] **Step 2: Update BufferSection.cpp initializer list**

Remove `recModeAtt` from the initializer list:

```cpp
BufferSection::BufferSection(juce::AudioProcessorValueTreeState& apvts)
    : lenAtt (apvts, "buffer_length_ms", bufferLength.getSlider()),
      fbAtt  (apvts, "record_feedback",  recordFeedback.getSlider()),
      gainAtt(apvts, "input_gain_db",    inputGain.getSlider())
{
```

- [ ] **Step 3: Add items and create attachment in constructor body**

After `recordModeLabel.setText(...)` and before `addAndMakeVisible` calls:

```cpp
    recordModeBox.addItemList({"Off", "Continuous", "One-Shot", "Gate"}, 1);
    recModeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "record_mode", recordModeBox);
```

- [ ] **Step 4: Build and verify**

```bash
cmake --build build --config Release --target Granulator_Standalone
```

Buffer "Record Mode" dropdown should show Off / Continuous / One-Shot / Gate.

- [ ] **Step 5: Commit**

```bash
git add source/ui/sections/BufferSection.h source/ui/sections/BufferSection.cpp
git commit -m "fix: populate Buffer section record mode ComboBox"
```

---

## Task 4: Populate EffectsSection ComboBox

**Files:**
- Modify: `source/ui/sections/EffectsSection.cpp`

`wsTypeAtt` is already a `unique_ptr` and created in the constructor body (line 12) — just missing the `addItemList` call before it.

- [ ] **Step 1: Add items before attachment creation**

In `EffectsSection.cpp`, find the line:
```cpp
wsTypeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "fx_ws_type", wsTypeBox);
```

Insert `addItemList` immediately before it:

```cpp
    wsTypeBox.addItemList({"Tanh", "Soft Clip"}, 1);
    wsTypeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "fx_ws_type", wsTypeBox);
```

- [ ] **Step 2: Build and verify**

```bash
cmake --build build --config Release --target Granulator_Standalone
```

Effects "WS Type" dropdown should show Tanh / Soft Clip.

- [ ] **Step 3: Commit**

```bash
git add source/ui/sections/EffectsSection.cpp
git commit -m "fix: populate Effects section waveshaper type ComboBox"
```

---

## Task 5: Fix Record Button Clipping

**Files:**
- Modify: `source/PluginEditor.cpp`

**Context:** In `PluginEditor::resized()` (around line 200), RecordSection is allocated `kBodyH - fxH` height where `fxH = kBodyH * 0.75`. That gives RecordSection ≈151px. Its internal layout places the REC and OPEN FOLDER buttons at y=136 with height=32, requiring at least 168px. Fix: change fxH proportion from 0.75 to 0.65, giving RecordSection ≈211px.

- [ ] **Step 1: Update the fxH proportion**

In `PluginEditor.cpp` `resized()`, around line 200, change:

```cpp
// Before:
const int fxH   = static_cast<int>(kBodyH * 0.75f);

// After:
const int fxH   = static_cast<int>(kBodyH * 0.65f);
```

Everything else in that block is computed relative to `fxH` and `recH`, so no other changes needed.

- [ ] **Step 2: Build and verify**

```bash
cmake --build build --config Release --target Granulator_Standalone
```

The REC button and OPEN FOLDER button should both be fully visible in the bottom-right of the plugin. The Effects section above it will be slightly shorter — confirm nothing is cut off there.

- [ ] **Step 3: Commit**

```bash
git add source/PluginEditor.cpp
git commit -m "fix: increase RecordSection height to expose REC and OPEN FOLDER buttons"
```

---

## Task 6: Add grain_probability Knob to PlayheadSection

**Files:**
- Modify: `source/ui/sections/PlayheadSection.h`
- Modify: `source/ui/sections/PlayheadSection.cpp`

**Layout plan:** Row 2 of PlayheadSection (y=98, height=82) currently has the scan shape combo and freeze toggle, leaving horizontal space from x≈214 to x≈274. The `prob` knob fits there at width=60.

- [ ] **Step 1: Update PlayheadSection.h**

Add `prob` to the `KnobWithLabel` list (line 13) and add `probAtt` to the `SliderAttachment` group (line 18):

```cpp
// Line 13 — add prob:
KnobWithLabel position, spray, scanRate, loopStart, loopEnd, scrub, prob;

// Line 18 — add probAtt:
juce::AudioProcessorValueTreeState::SliderAttachment
    posAtt, sprayAtt, scanRateAtt, loopStartAtt, loopEndAtt, scrubAtt, probAtt;
```

- [ ] **Step 2: Update PlayheadSection.cpp initializer list**

Add `probAtt` to the initializer list:

```cpp
PlayheadSection::PlayheadSection(juce::AudioProcessorValueTreeState& apvts)
    : posAtt      (apvts, "position",          position.getSlider()),
      sprayAtt    (apvts, "spray_ms",           spray.getSlider()),
      scanRateAtt (apvts, "scan_rate_hz",       scanRate.getSlider()),
      loopStartAtt(apvts, "loop_start",         loopStart.getSlider()),
      loopEndAtt  (apvts, "loop_end",           loopEnd.getSlider()),
      scrubAtt    (apvts, "scrub",              scrub.getSlider()),
      probAtt     (apvts, "grain_probability",  prob.getSlider()),
      freezeAtt   (apvts, "freeze",             freezeBtn)
{
```

- [ ] **Step 3: Set label and make visible in constructor body**

After `scrub.setLabel("Scrub");` (around line 19), add:

```cpp
    prob.setLabel("Prob");
```

After `addAndMakeVisible(freezeBtn);` (around line 26), add:

```cpp
    addAndMakeVisible(prob);
```

- [ ] **Step 4: Add prob to resized()**

In `PlayheadSection::resized()`, after the `freezeBtn.setBounds(...)` line (line 41), add:

```cpp
    prob.setBounds(214, row2Y, 60, kH);
```

Full updated `resized()` for reference:

```cpp
void PlayheadSection::resized()
{
    const int kW = 70, kH = 82, gap = 8, startX = 8, startY = 8;
    const int row2Y = startY + kH + gap;
    const int row3Y = row2Y + kH + gap;

    int x = startX;
    for (auto* c : { &position, &spray, &scanRate })
    { c->setBounds(x, startY, kW, kH); x += kW + gap; }

    x = startX;
    scanShapeLabel.setBounds(x, row2Y, 120, 14);
    scanShapeBox.setBounds(x, row2Y + 16, 120, 22);
    freezeBtn.setBounds(x + 130, row2Y + 16, 70, 28);
    prob.setBounds(214, row2Y, 60, kH);

    x = startX;
    for (auto* c : { &loopStart, &loopEnd, &scrub })
    { c->setBounds(x, row3Y, kW, kH); x += kW + gap; }
}
```

- [ ] **Step 5: Build and verify**

```bash
cmake --build build --config Release --target Granulator_Standalone
```

A "Prob" knob (0–1 range) should appear in the Playhead section, to the right of the Freeze toggle. Moving it should not crash.

- [ ] **Step 6: Commit**

```bash
git add source/ui/sections/PlayheadSection.h source/ui/sections/PlayheadSection.cpp
git commit -m "feat: add grain_probability knob to PlayheadSection"
```

---

## Task 7: Add EnvModStrip to ModSection

**Files:**
- Modify: `source/ui/sections/ModSection.h`
- Modify: `source/ui/sections/ModSection.cpp`

**Context:** ModSection needs 2 Env Mod strips (each with Atk/Dec/Sus/Rel knobs) and 1 Follower strip (Atk/Rel knobs). These stack below the 4 LFO strips. Because total content height exceeds the available screen area, a `juce::Viewport` wraps a content component that holds everything. The Viewport provides vertical scroll.

- [ ] **Step 1: Update ModSection.h**

Replace the entire file with:

```cpp
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../components/KnobWithLabel.h"
#include "../components/ModMatrixGrid.h"

class ModSection : public juce::Component
{
public:
    ModSection(juce::AudioProcessorValueTreeState& apvts, ModMatrix& modMatrix);
    void resized() override;

private:
    struct LFOStrip : public juce::Component
    {
        explicit LFOStrip(juce::AudioProcessorValueTreeState& apvts, int n);
        void resized() override;

        KnobWithLabel rate, depth, phase;
        juce::ComboBox shapeBox;
        juce::Label    label, shapeLabel;

        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rateAtt, depthAtt, phaseAtt;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> shapeAtt;
    };

    struct EnvModStrip : public juce::Component
    {
        explicit EnvModStrip(juce::AudioProcessorValueTreeState& apvts, int n);
        void resized() override;

        KnobWithLabel attack, decay, sustain, release;
        juce::Label   label;

        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
            atkAtt, decAtt, susAtt, relAtt;
    };

    std::array<std::unique_ptr<LFOStrip>, 4>    lfoStrips;
    std::array<std::unique_ptr<EnvModStrip>, 2>  envStrips;

    // Envelope Follower (inline — only 2 knobs)
    KnobWithLabel efAttack, efRelease;
    juce::Label   efLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> efAtkAtt, efRelAtt;

    // Section labels
    juce::Label lfoSectionLabel, envSectionLabel, followerSectionLabel, matrixSectionLabel;

    ModMatrixGrid        modGrid;
    juce::Component      contentComp;   // scrollable content host
    juce::Viewport       viewport;
};
```

- [ ] **Step 2: Implement EnvModStrip in ModSection.cpp**

Add the `EnvModStrip` constructor and `resized()` after the existing `LFOStrip::resized()` (after line 37), before the `ModSection::ModSection` constructor:

```cpp
ModSection::EnvModStrip::EnvModStrip(juce::AudioProcessorValueTreeState& apvts, int n)
{
    auto ns = juce::String(n);
    label.setText("ENV " + ns, juce::dontSendNotification);
    attack.setLabel("Atk");
    decay.setLabel("Dec");
    sustain.setLabel("Sus");
    release.setLabel("Rel");

    atkAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "envmod" + ns + "_attack_ms",  attack.getSlider());
    decAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "envmod" + ns + "_decay_ms",   decay.getSlider());
    susAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "envmod" + ns + "_sustain",    sustain.getSlider());
    relAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "envmod" + ns + "_release_ms", release.getSlider());

    addAndMakeVisible(label);
    addAndMakeVisible(attack);
    addAndMakeVisible(decay);
    addAndMakeVisible(sustain);
    addAndMakeVisible(release);
}

void ModSection::EnvModStrip::resized()
{
    label.setBounds(0, 0, 60, 14);
    const int kW = 62, kH = getHeight() - 18, gap = 5;
    int x = 0;
    attack.setBounds (x, 18, kW, kH); x += kW + gap;
    decay.setBounds  (x, 18, kW, kH); x += kW + gap;
    sustain.setBounds(x, 18, kW, kH); x += kW + gap;
    release.setBounds(x, 18, kW, kH);
}
```

- [ ] **Step 3: Update ModSection constructor**

Replace the existing `ModSection::ModSection` constructor (lines 39–48) with:

```cpp
ModSection::ModSection(juce::AudioProcessorValueTreeState& apvts, ModMatrix& modMatrix)
    : modGrid(modMatrix)
{
    for (int i = 0; i < 4; ++i)
    {
        lfoStrips[i] = std::make_unique<LFOStrip>(apvts, i + 1);
        contentComp.addAndMakeVisible(*lfoStrips[i]);
    }

    for (int i = 0; i < 2; ++i)
    {
        envStrips[i] = std::make_unique<EnvModStrip>(apvts, i + 1);
        contentComp.addAndMakeVisible(*envStrips[i]);
    }

    efLabel.setText("FOLLOWER", juce::dontSendNotification);
    efAttack.setLabel("Atk");
    efRelease.setLabel("Rel");
    efAtkAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "ef_attack_ms",  efAttack.getSlider());
    efRelAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "ef_release_ms", efRelease.getSlider());
    contentComp.addAndMakeVisible(efLabel);
    contentComp.addAndMakeVisible(efAttack);
    contentComp.addAndMakeVisible(efRelease);

    lfoSectionLabel.setText("LFOs", juce::dontSendNotification);
    envSectionLabel.setText("Envelope Mods", juce::dontSendNotification);
    followerSectionLabel.setText("Follower", juce::dontSendNotification);
    matrixSectionLabel.setText("Mod Matrix", juce::dontSendNotification);
    for (auto* l : { &lfoSectionLabel, &envSectionLabel,
                     &followerSectionLabel, &matrixSectionLabel })
    {
        l->setFont(juce::Font(10.f, juce::Font::bold));
        l->setColour(juce::Label::textColourId, juce::Colour(0xff888888));
        contentComp.addAndMakeVisible(l);
    }

    contentComp.addAndMakeVisible(modGrid);

    viewport.setViewedComponent(&contentComp, false);
    viewport.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport);
}
```

- [ ] **Step 4: Update ModSection::resized()**

Replace the existing `resized()` (lines 50–59) with:

```cpp
void ModSection::resized()
{
    viewport.setBounds(getLocalBounds());

    const int w        = getWidth() - 16;  // strip width (viewport inset)
    const int stripH   = 110;              // LFO strip height (unchanged)
    const int envH     = 90;              // EnvMod strip height
    const int followerH= 90;              // Follower knob row height
    const int matrixH  = 220;             // ModMatrixGrid fixed height
    const int sectionLblH = 16;
    const int gap      = 6;

    // Compute total content height
    const int totalH =
        sectionLblH + 4 * (stripH + gap) +        // LFOs
        sectionLblH + 2 * (envH  + gap) +          // EnvMods
        sectionLblH + followerH + gap +             // Follower
        sectionLblH + matrixH + gap;                // Matrix

    contentComp.setBounds(0, 0, getWidth(), totalH);

    int y = 4;

    // LFO section
    lfoSectionLabel.setBounds(4, y, w, sectionLblH);
    y += sectionLblH + 2;
    for (auto& strip : lfoStrips)
    {
        strip->setBounds(4, y, w, stripH);
        y += stripH + gap;
    }

    // EnvMod section
    envSectionLabel.setBounds(4, y, w, sectionLblH);
    y += sectionLblH + 2;
    for (auto& strip : envStrips)
    {
        strip->setBounds(4, y, w, envH);
        y += envH + gap;
    }

    // Follower section
    followerSectionLabel.setBounds(4, y, w, sectionLblH);
    y += sectionLblH + 2;
    efLabel.setBounds(4, y, 80, 14);
    const int kW = 70, kH = followerH - 18;
    efAttack.setBounds (4,        y + 18, kW, kH);
    efRelease.setBounds(4 + kW + gap, y + 18, kW, kH);
    y += followerH + gap;

    // Matrix section
    matrixSectionLabel.setBounds(4, y, w, sectionLblH);
    y += sectionLblH + 2;
    modGrid.setBounds(4, y, w, matrixH);
}
```

- [ ] **Step 5: Build and verify**

```bash
cmake --build build --config Release --target Granulator_Standalone
```

The Mod column should now scroll vertically. Below the 4 LFO strips: two ENV strips (Atk/Dec/Sus/Rel each), a Follower strip (Atk/Rel), and the Mod Matrix grid. All knobs should respond to mouse interaction.

- [ ] **Step 6: Commit**

```bash
git add source/ui/sections/ModSection.h source/ui/sections/ModSection.cpp
git commit -m "feat: add Envelope Mod and Follower strips to ModSection with scrollable viewport"
```

---

## Task 8: Final Integration Check

- [ ] **Step 1: Full build**

```bash
cmake --build build --config Release
```

All targets (VST3 + Standalone) should compile clean with no warnings about unused parameters or missing overrides.

- [ ] **Step 2: Smoke test checklist**

Launch `build/Granulator_artefacts/Release/Standalone/Granulator.exe` and verify:

| Check | Expected |
|---|---|
| Pitch → Transpose dropdown | Shows: Doppler, Interpolated, Phase Vocoder |
| Pitch → Quantize dropdown | Shows: Off, Chromatic, Major, Minor, Custom |
| Filter → Type dropdown | Shows: Low-Pass, High-Pass, Band-Pass, Notch |
| Buffer → Record Mode dropdown | Shows: Off, Continuous, One-Shot, Gate |
| Effects → WS Type dropdown | Shows: Tanh, Soft Clip |
| Playhead section | "Prob" knob visible next to Freeze toggle |
| Column 4 bottom | REC button and OPEN FOLDER button fully visible |
| Mod column scroll | Scrolls to reveal ENV 1, ENV 2, Follower, and Matrix sections |
| Env Mod knobs | All 8 knobs (2 strips × 4) respond to drag |
| Follower knobs | Both Atk and Rel knobs respond to drag |

- [ ] **Step 3: Commit if any final tweaks were needed**

```bash
git add -p
git commit -m "fix: integration cleanup after functional fix pass"
```
