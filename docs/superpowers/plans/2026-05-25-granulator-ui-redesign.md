# Granulator UI Redesign Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the 10-tab UI with a single 1400×920 column-grid master view, add grain probability DSP parameter, add a preset browser, and wire logo-click to randomize all parameters.

**Architecture:** Five-column flat layout — Grain/Playhead | Pitch/Amplitude | Space/Filter/Buffer/Record | FX | LFO Routing Panel. TabbedComponent removed entirely. New PresetManager handles file I/O; PresetBar lives in the header. LFORoutingPanel owns all LFO parameter attachments and displays live routing from ModMatrix.

**Tech Stack:** C++17 / JUCE 7.0.12 / CMake + Ninja. Build: `cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release` from project root.

---

## File Map

| Action | File |
|--------|------|
| Modify | `source/PluginProcessor.h` — add `grainProbability` to RawParams |
| Modify | `source/PluginProcessor.cpp` — add param to layout, cache, buildParamSnapshot |
| Modify | `source/dsp/GranularParams.h` — add `grainProbability` field |
| Modify | `source/grain/GranularEngine.cpp` — probability gate in scheduler |
| Modify | `source/ui/GranularLookAndFeel.h/.cpp` — contrast fixes, window constants |
| Modify | `source/ui/sections/GrainCoreSection.h/.cpp` — add PROB knob, new layout |
| Modify | `source/ui/sections/PlayheadSection.h/.cpp` — resized() for 270px column |
| Modify | `source/ui/sections/PitchSection.h/.cpp` — resized() for 270px column |
| Modify | `source/ui/sections/AmplitudeSection.h/.cpp` — resized() for 270px column |
| Modify | `source/ui/sections/SpatialSection.h/.cpp` — resized() for 270px column |
| Modify | `source/ui/sections/FilterSection.h/.cpp` — resized() for 270px column |
| Modify | `source/ui/sections/BufferSection.h/.cpp` — resized() for 270px column |
| Modify | `source/ui/sections/EffectsSection.h/.cpp` — resized() for 270px column |
| Modify | `source/ui/sections/RecordSection.h/.cpp` — resized() for 270px column |
| Create | `source/ui/PresetManager.h/.cpp` — file I/O for .granpreset files |
| Create | `source/ui/components/PresetBar.h/.cpp` — header preset nav + save |
| Create | `source/ui/LFORoutingPanel.h/.cpp` — 4 LFO strips + routing display + output |
| Modify | `source/PluginEditor.h/.cpp` — full rewrite: flat layout, randomize, new components |

---

## Task 1: grain_probability — Parameter + DSP

**Files:**
- Modify: `source/dsp/GranularParams.h`
- Modify: `source/PluginProcessor.h:54-146`
- Modify: `source/PluginProcessor.cpp:20-146` (layout), `169-272` (cache), `302+` (buildSnapshot)
- Modify: `source/grain/GranularEngine.cpp:222-229`

- [ ] **Step 1: Add field to GranularParams**

In `source/dsp/GranularParams.h`, add after line 44 (`randomizeDensity`):
```cpp
    float grainProbability  = 1.0f;    // 0..1, chance a scheduled grain fires
```

- [ ] **Step 2: Add APVTS parameter**

In `source/PluginProcessor.cpp`, add after line 38 (after `randomize_density` layout.add):
```cpp
    layout.add(std::make_unique<P>("grain_probability", "Grain Probability",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f));
```

- [ ] **Step 3: Add to RawParams struct**

In `source/PluginProcessor.h`, add after line 64 (`randomizeDensity`):
```cpp
        std::atomic<float>* grainProbability  = nullptr;
```

- [ ] **Step 4: Cache the raw pointer**

In `source/PluginProcessor.cpp` in `cacheRawParams()`, add after the `randomizeDensity` cache line:
```cpp
    r->grainProbability  = apvts.getRawParameterValue("grain_probability");
```

- [ ] **Step 5: Wire into buildParamSnapshot**

In `source/PluginProcessor.cpp` in `buildParamSnapshot()`, add after the `randomizeDensity` snapshot line:
```cpp
    p.grainProbability  = raw.grainProbability->load();
```

- [ ] **Step 6: Add probability gate in GranularEngine**

In `source/grain/GranularEngine.cpp`, replace lines 226-228:
```cpp
        if (--schedulerCountdown <= 0.0)
        {
            spawnGrain(params, buffer, bufLen);
            schedulerCountdown = computeInteronsetSamples(params);
        }
```
With:
```cpp
        if (--schedulerCountdown <= 0.0)
        {
            if (params.grainProbability >= 1.0f || rng.nextFloat() < params.grainProbability)
                spawnGrain(params, buffer, bufLen);
            schedulerCountdown = computeInteronsetSamples(params);
        }
```

- [ ] **Step 7: Build and verify**

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release
```
Expected: build succeeds with no errors.

- [ ] **Step 8: Commit**
```bash
git add source/dsp/GranularParams.h source/PluginProcessor.h source/PluginProcessor.cpp source/grain/GranularEngine.cpp
git commit -m "feat: add grain_probability parameter and DSP gate"
```

---

## Task 2: GranularLookAndFeel — Contrast Fixes + Window Constants

**Files:**
- Modify: `source/ui/GranularLookAndFeel.h`
- Modify: `source/ui/GranularLookAndFeel.cpp`

- [ ] **Step 1: Update colour palette and add window constants in header**

Replace the entire colour palette block and class declaration in `source/ui/GranularLookAndFeel.h` with:
```cpp
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Window geometry constants used by PluginEditor and sections
static constexpr int kWindowW  = 1400;
static constexpr int kWindowH  = 920;
static constexpr int kHeaderH  = 40;
static constexpr int kEdgePad  = 8;
static constexpr int kColGap   = 8;
static constexpr int kColW     = 270;   // (1400 - 2*8 - 4*8) / 5
static constexpr int kKnobW    = 76;
static constexpr int kKnobH    = 88;
static constexpr int kKnobGap  = 9;

class GranularLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GranularLookAndFeel();

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override;

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    // Colour palette
    static const juce::Colour backgroundDark;   // #0c0c0c
    static const juce::Colour backgroundMid;    // #141414
    static const juce::Colour panelDark;        // #0a0a0a  (LFO panel bg)
    static const juce::Colour accent;           // #ffffff
    static const juce::Colour textPrimary;      // #cccccc  (knob labels)
    static const juce::Colour textSecondary;    // #aaaaaa  (section labels)
    static const juce::Colour textDim;          // #666666  (routing depth values)
    static const juce::Colour textMuted;        // #444444  (unpatched state)
    static const juce::Colour divider;          // #1e1e1e  (column/section lines)
    static const juce::Colour knobBody;         // #202020
    static const juce::Colour knobThumb;        // #ffffff
};
```

- [ ] **Step 2: Update colour definitions in .cpp**

In `source/ui/GranularLookAndFeel.cpp`, replace all static colour definitions at the top with:
```cpp
const juce::Colour GranularLookAndFeel::backgroundDark  = juce::Colour(0xff0c0c0c);
const juce::Colour GranularLookAndFeel::backgroundMid   = juce::Colour(0xff141414);
const juce::Colour GranularLookAndFeel::panelDark       = juce::Colour(0xff0a0a0a);
const juce::Colour GranularLookAndFeel::accent          = juce::Colour(0xffffffff);
const juce::Colour GranularLookAndFeel::textPrimary     = juce::Colour(0xffcccccc);
const juce::Colour GranularLookAndFeel::textSecondary   = juce::Colour(0xffaaaaaa);
const juce::Colour GranularLookAndFeel::textDim         = juce::Colour(0xff666666);
const juce::Colour GranularLookAndFeel::textMuted       = juce::Colour(0xff444444);
const juce::Colour GranularLookAndFeel::divider         = juce::Colour(0xff1e1e1e);
const juce::Colour GranularLookAndFeel::knobBody        = juce::Colour(0xff202020);
const juce::Colour GranularLookAndFeel::knobThumb       = juce::Colour(0xffffffff);
```

- [ ] **Step 3: Remove tab drawing overrides from .cpp**

Delete the `drawTabButton` and `drawTabAreaBehindFrontButton` method implementations from `GranularLookAndFeel.cpp` — they are no longer needed. Also remove their declarations from the header in Step 1 (already done above).

- [ ] **Step 4: Update Label colour IDs in constructor**

In `GranularLookAndFeel` constructor in .cpp, update the label-related colour setters to use the new constants:
```cpp
setColour(juce::Label::textColourId,            textPrimary);
setColour(juce::ComboBox::textColourId,         textPrimary);
setColour(juce::ComboBox::backgroundColourId,   backgroundMid);
setColour(juce::ComboBox::outlineColourId,      divider);
setColour(juce::ToggleButton::textColourId,     textPrimary);
setColour(juce::ToggleButton::tickColourId,     accent);
setColour(juce::TextButton::buttonColourId,     backgroundMid);
setColour(juce::TextButton::textColourOffId,    textPrimary);
setColour(juce::PopupMenu::backgroundColourId,  backgroundMid);
setColour(juce::PopupMenu::textColourId,        textPrimary);
setColour(juce::PopupMenu::highlightedBackgroundColourId, accent.withAlpha(0.12f));
setColour(juce::PopupMenu::highlightedTextColourId,       accent);
```

- [ ] **Step 5: Build**
```bash
cmake --build build --config Release
```
Expected: compiles clean. Fix any reference to removed colour names (e.g. `textSecondary` used in sections — those will be fixed in later tasks).

- [ ] **Step 6: Commit**
```bash
git add source/ui/GranularLookAndFeel.h source/ui/GranularLookAndFeel.cpp
git commit -m "style: update LAF contrast colours, add window geometry constants"
```

---

## Task 3: GrainCoreSection — Add PROB Knob + 270px Layout

**Files:**
- Modify: `source/ui/sections/GrainCoreSection.h`
- Modify: `source/ui/sections/GrainCoreSection.cpp`

- [ ] **Step 1: Update header**

Replace `source/ui/sections/GrainCoreSection.h` entirely:
```cpp
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../components/KnobWithLabel.h"

class GrainCoreSection : public juce::Component
{
public:
    explicit GrainCoreSection(juce::AudioProcessorValueTreeState& apvts);
    void resized() override;

private:
    KnobWithLabel grainSize, grainDensity, grainOverlap,
                  interonset, randomizeSize, randomizeDensity, probability;
    juce::ComboBox envelopeBox, directionBox;
    juce::Label    envelopeLabel, directionLabel;

    juce::AudioProcessorValueTreeState::SliderAttachment
        sizeAtt, densityAtt, overlapAtt, interonsetAtt,
        randSizeAtt, randDensityAtt, probabilityAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> envAtt, dirAtt;
};
```

- [ ] **Step 2: Update constructor and resized()**

Replace `source/ui/sections/GrainCoreSection.cpp` entirely:
```cpp
#include "GrainCoreSection.h"
#include "../GranularLookAndFeel.h"

GrainCoreSection::GrainCoreSection(juce::AudioProcessorValueTreeState& apvts)
    : sizeAtt        (apvts, "grain_size_ms",     grainSize.getSlider()),
      densityAtt     (apvts, "grain_density",     grainDensity.getSlider()),
      overlapAtt     (apvts, "grain_overlap",     grainOverlap.getSlider()),
      interonsetAtt  (apvts, "interonset_ms",     interonset.getSlider()),
      randSizeAtt    (apvts, "randomize_size_ms", randomizeSize.getSlider()),
      randDensityAtt (apvts, "randomize_density", randomizeDensity.getSlider()),
      probabilityAtt (apvts, "grain_probability", probability.getSlider())
{
    grainSize.setLabel("Size");
    grainDensity.setLabel("Density");
    grainOverlap.setLabel("Overlap");
    interonset.setLabel("Interonset");
    randomizeSize.setLabel("Rnd Size");
    randomizeDensity.setLabel("Rnd Dens");
    probability.setLabel("Prob");

    envelopeLabel.setText("Envelope", juce::dontSendNotification);
    envelopeLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    envelopeLabel.setColour(juce::Label::textColourId, GranularLookAndFeel::textSecondary);
    directionLabel.setText("Direction", juce::dontSendNotification);
    directionLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    directionLabel.setColour(juce::Label::textColourId, GranularLookAndFeel::textSecondary);

    envelopeBox.addItemList({"Hann","Gaussian","Trapezoid","Rectangle","Triangular","Tukey"}, 1);
    directionBox.addItemList({"Forward","Reverse","Bidirectional","Random"}, 1);
    envAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "grain_envelope", envelopeBox);
    dirAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "grain_direction", directionBox);

    for (auto* c : { &grainSize, &grainDensity, &grainOverlap,
                     &interonset, &randomizeSize, &randomizeDensity, &probability })
        addAndMakeVisible(c);
    addAndMakeVisible(envelopeBox);
    addAndMakeVisible(directionBox);
    addAndMakeVisible(envelopeLabel);
    addAndMakeVisible(directionLabel);
}

void GrainCoreSection::resized()
{
    // 3 knobs per row in 270px column
    const int kw = kKnobW, kh = kKnobH, gap = kKnobGap;
    const int pad = 6;
    int y = pad;

    // Row 1: Size, Density, Overlap
    grainSize.setBounds     (pad + 0*(kw+gap), y, kw, kh);
    grainDensity.setBounds  (pad + 1*(kw+gap), y, kw, kh);
    grainOverlap.setBounds  (pad + 2*(kw+gap), y, kw, kh);
    y += kh + gap;

    // Row 2: Interonset, Rnd Size, Rnd Density
    interonset.setBounds      (pad + 0*(kw+gap), y, kw, kh);
    randomizeSize.setBounds   (pad + 1*(kw+gap), y, kw, kh);
    randomizeDensity.setBounds(pad + 2*(kw+gap), y, kw, kh);
    y += kh + gap;

    // Row 3: Probability + combos
    probability.setBounds(pad, y, kw, kh);

    const int comboW = kw + 10;
    const int comboH = 20;
    const int comboX = pad + kw + gap;
    envelopeLabel.setBounds(comboX, y, comboW, 13);
    envelopeBox.setBounds  (comboX, y + 14, comboW, comboH);
    directionLabel.setBounds(comboX + comboW + 4, y, comboW, 13);
    directionBox.setBounds  (comboX + comboW + 4, y + 14, comboW, comboH);
}
```

- [ ] **Step 3: Build**
```bash
cmake --build build --config Release
```
Expected: compiles clean.

- [ ] **Step 4: Commit**
```bash
git add source/ui/sections/GrainCoreSection.h source/ui/sections/GrainCoreSection.cpp
git commit -m "feat: add grain probability knob to GrainCoreSection, 270px layout"
```

---

## Task 4: Update Remaining Section Layouts for 270px Columns

Each section needs its `resized()` rewritten to work in a 270px wide column. The sections themselves (members, constructors, attachments) are unchanged — only the layout.

**Files:** All `.cpp` files in `source/ui/sections/` except GrainCoreSection (already done).

- [ ] **Step 1: PlayheadSection::resized()**

In `source/ui/sections/PlayheadSection.cpp`, replace `resized()`:
```cpp
void PlayheadSection::resized()
{
    const int kw = kKnobW, kh = kKnobH, gap = kKnobGap, pad = 6;
    int y = pad;
    // Row 1: Position, Spray, Scan Rate
    positionKnob.setBounds (pad + 0*(kw+gap), y, kw, kh);
    sprayKnob.setBounds    (pad + 1*(kw+gap), y, kw, kh);
    scanRateKnob.setBounds (pad + 2*(kw+gap), y, kw, kh);
    y += kh + gap;
    // Row 2: Loop Start, Loop End, Scrub
    loopStartKnob.setBounds(pad + 0*(kw+gap), y, kw, kh);
    loopEndKnob.setBounds  (pad + 1*(kw+gap), y, kw, kh);
    scrubKnob.setBounds    (pad + 2*(kw+gap), y, kw, kh);
    y += kh + gap;
    // Row 3: scan shape combo + freeze toggle
    const int comboW = 120, comboH = 20;
    scanShapeLabel.setBounds(pad, y, comboW, 13);
    scanShapeBox.setBounds  (pad, y + 14, comboW, comboH);
    freezeButton.setBounds  (pad + comboW + 8, y + 10, 70, 24);
}
```

> **Note:** Adapt member names to match what actually exists in PlayheadSection.h. The pattern (3 per row, combos/toggles at bottom) is the same for all sections.

- [ ] **Step 2: PitchSection::resized()**

In `source/ui/sections/PitchSection.cpp`, replace `resized()`:
```cpp
void PitchSection::resized()
{
    const int kw = kKnobW, kh = kKnobH, gap = kKnobGap, pad = 6;
    int y = pad;
    // Row 1: Shift, Detune, Rnd
    pitchShiftKnob.setBounds (pad + 0*(kw+gap), y, kw, kh);
    detuneKnob.setBounds     (pad + 1*(kw+gap), y, kw, kh);
    pitchRndKnob.setBounds   (pad + 2*(kw+gap), y, kw, kh);
    y += kh + gap;
    // Row 2: Rate, Formant, Glide
    playbackRateKnob.setBounds(pad + 0*(kw+gap), y, kw, kh);
    formantKnob.setBounds    (pad + 1*(kw+gap), y, kw, kh);
    glideKnob.setBounds      (pad + 2*(kw+gap), y, kw, kh);
    y += kh + gap;
    // Row 3: Transpose + Quantize combos
    const int comboW = 120, comboH = 20;
    transposeModeLabel.setBounds (pad, y, comboW, 13);
    transposeModeBox.setBounds   (pad, y + 14, comboW, comboH);
    pitchQuantizeLabel.setBounds (pad + comboW + 8, y, comboW, 13);
    pitchQuantizeBox.setBounds   (pad + comboW + 8, y + 14, comboW, comboH);
}
```

- [ ] **Step 3: AmplitudeSection::resized()**

In `source/ui/sections/AmplitudeSection.cpp`, replace `resized()`:
```cpp
void AmplitudeSection::resized()
{
    const int kw = kKnobW, kh = kKnobH, gap = kKnobGap, pad = 6;
    int y = pad;
    // Row 1: Amp, Rnd, VelSens, Xfade
    ampKnob.setBounds    (pad + 0*(kw+gap), y, kw, kh);
    ampRndKnob.setBounds (pad + 1*(kw+gap), y, kw, kh);
    velSensKnob.setBounds(pad + 2*(kw+gap), y, kw, kh);
    y += kh + gap;
    // Row 2: Attack, Decay, Sustain
    attackKnob.setBounds (pad + 0*(kw+gap), y, kw, kh);
    decayKnob.setBounds  (pad + 1*(kw+gap), y, kw, kh);
    sustainKnob.setBounds(pad + 2*(kw+gap), y, kw, kh);
    y += kh + gap;
    // Row 3: Release, Crossfade
    releaseKnob.setBounds  (pad + 0*(kw+gap), y, kw, kh);
    crossfadeKnob.setBounds(pad + 1*(kw+gap), y, kw, kh);
}
```

- [ ] **Step 4: SpatialSection::resized()**

In `source/ui/sections/SpatialSection.cpp`, replace `resized()`:
```cpp
void SpatialSection::resized()
{
    const int kw = kKnobW, kh = kKnobH, gap = kKnobGap, pad = 6;
    int y = pad;
    // Row 1: Pan, Pan Rnd, Width
    panKnob.setBounds      (pad + 0*(kw+gap), y, kw, kh);
    panRndKnob.setBounds   (pad + 1*(kw+gap), y, kw, kh);
    widthKnob.setBounds    (pad + 2*(kw+gap), y, kw, kh);
    y += kh + gap;
    // Row 2: Voices, Voice Detune
    voicesKnob.setBounds      (pad + 0*(kw+gap), y, kw, kh);
    voiceDetuneKnob.setBounds (pad + 1*(kw+gap), y, kw, kh);
}
```

- [ ] **Step 5: FilterSection::resized()**

In `source/ui/sections/FilterSection.cpp`, replace `resized()`:
```cpp
void FilterSection::resized()
{
    const int kw = kKnobW, kh = kKnobH, gap = kKnobGap, pad = 6;
    int y = pad;
    // Filter type combo
    const int comboH = 20;
    filterTypeLabel.setBounds(pad, y, 120, 13);
    filterTypeBox.setBounds  (pad, y + 14, 130, comboH);
    y += 14 + comboH + gap;
    // Row 1: Cutoff, Resonance, Keytrack
    cutoffKnob.setBounds   (pad + 0*(kw+gap), y, kw, kh);
    resonanceKnob.setBounds(pad + 1*(kw+gap), y, kw, kh);
    keytrackKnob.setBounds (pad + 2*(kw+gap), y, kw, kh);
    y += kh + gap;
    // Row 2: Env Depth, LFO Depth
    envDepthKnob.setBounds(pad + 0*(kw+gap), y, kw, kh);
    lfoDepthKnob.setBounds(pad + 1*(kw+gap), y, kw, kh);
}
```

- [ ] **Step 6: BufferSection::resized()**

In `source/ui/sections/BufferSection.cpp`, replace `resized()`:
```cpp
void BufferSection::resized()
{
    const int kw = kKnobW, kh = kKnobH, gap = kKnobGap, pad = 6;
    int y = pad;
    // Waveform display: full width, fixed height
    const int wfH = 140;
    waveformDisplay.setBounds(pad, y, getWidth() - pad*2, wfH);
    y += wfH + gap;
    // Row: Length, Feedback, Gain
    bufferLengthKnob.setBounds(pad + 0*(kw+gap), y, kw, kh);
    feedbackKnob.setBounds    (pad + 1*(kw+gap), y, kw, kh);
    inputGainKnob.setBounds   (pad + 2*(kw+gap), y, kw, kh);
    y += kh + gap;
    // Record mode combo
    const int comboH = 20;
    recordModeLabel.setBounds(pad, y, 120, 13);
    recordModeBox.setBounds  (pad, y + 14, 130, comboH);
    y += 14 + comboH + gap;
    // Buttons
    loadFileButton.setBounds(pad, y, 120, 24);
    clearButton.setBounds   (pad + 128, y, 70, 24);
}
```

- [ ] **Step 7: EffectsSection::resized()**

In `source/ui/sections/EffectsSection.cpp`, replace `resized()`:
```cpp
void EffectsSection::resized()
{
    const int kw = kKnobW, kh = kKnobH, gap = kKnobGap, pad = 6;
    const int lineH = 1, sectionGap = 8;
    int y = pad;

    // Waveshaper: Drive knob + type combo side by side
    driveKnob.setBounds(pad, y, kw, kh);
    wsTypeLabel.setBounds  (pad + kw + gap, y, 90, 13);
    wsTypeBox.setBounds    (pad + kw + gap, y + 14, 100, 20);
    y += kh + sectionGap;

    // EQ: 4 bands × 3 knobs in a 4-col sub-grid (tiny knobs)
    // Use kw-6 to fit 4 cols: 4*(kw-6) + 3*4 = 4*70+12 = 292 (too wide for 270)
    // Use 60px knobs for EQ: 4*60 + 3*5 = 255 fits
    const int eqKw = 58, eqKh = 70, eqGap = 5;
    // FREQ row
    for (int i = 0; i < 4; ++i)
        eqFreqKnobs[i].setBounds(pad + i*(eqKw+eqGap), y, eqKw, eqKh);
    y += eqKh + 2;
    // GAIN row
    for (int i = 0; i < 4; ++i)
        eqGainKnobs[i].setBounds(pad + i*(eqKw+eqGap), y, eqKw, eqKh);
    y += eqKh + 2;
    // Q row
    for (int i = 0; i < 4; ++i)
        eqQKnobs[i].setBounds(pad + i*(eqKw+eqGap), y, eqKw, eqKh);
    y += eqKh + sectionGap;

    // Chorus
    chorusRateKnob.setBounds (pad + 0*(kw+gap), y, kw, kh);
    chorusDepthKnob.setBounds(pad + 1*(kw+gap), y, kw, kh);
    chorusMixKnob.setBounds  (pad + 2*(kw+gap), y, kw, kh);
    y += kh + sectionGap;

    // Delay
    delayTimeKnob.setBounds    (pad + 0*(kw+gap), y, kw, kh);
    delayFeedbackKnob.setBounds(pad + 1*(kw+gap), y, kw, kh);
    delayMixKnob.setBounds     (pad + 2*(kw+gap), y, kw, kh);
    y += kh + 4;
    pingPongButton.setBounds(pad, y, 100, 20);
    y += 24 + sectionGap;

    // Reverb
    reverbRoomKnob.setBounds(pad + 0*(kw+gap), y, kw, kh);
    reverbDampKnob.setBounds(pad + 1*(kw+gap), y, kw, kh);
    reverbMixKnob.setBounds (pad + 2*(kw+gap), y, kw, kh);
    y += kh + sectionGap;

    // Limiter
    limiterThreshKnob.setBounds  (pad + 0*(kw+gap), y, kw, kh);
    limiterReleaseKnob.setBounds (pad + 1*(kw+gap), y, kw, kh);
}
```

> **Note:** Adapt member names (`eqFreqKnobs`, `eqGainKnobs`, `eqQKnobs`, etc.) to match the actual member names in EffectsSection.h. If EQ bands are stored as separate named members rather than arrays, lay them out individually following the same pattern.

- [ ] **Step 8: RecordSection::resized()**

In `source/ui/sections/RecordSection.cpp`, replace `resized()`:
```cpp
void RecordSection::resized()
{
    const int pad = 6;
    int y = pad;
    recButton.setBounds       (pad, y, 100, 28);
    openFolderButton.setBounds(pad + 108, y, 120, 28);
    y += 36;
    statusLabel.setBounds(pad, y, getWidth() - pad*2, 18);
    y += 20;
    timeLabel.setBounds  (pad, y, getWidth() - pad*2, 18);
    y += 20;
    fileLabel.setBounds  (pad, y, getWidth() - pad*2, 18);
}
```

- [ ] **Step 9: Build**
```bash
cmake --build build --config Release
```
Expected: compiles clean. Fix any name mismatches (member variable names differ from what's shown — look them up in the .h files).

- [ ] **Step 10: Commit**
```bash
git add source/ui/sections/
git commit -m "style: update all section layouts for 270px column width"
```

---

## Task 5: PresetManager

**Files:**
- Create: `source/ui/PresetManager.h`
- Create: `source/ui/PresetManager.cpp`

- [ ] **Step 1: Create PresetManager.h**

```cpp
// source/ui/PresetManager.h
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class PresetManager
{
public:
    explicit PresetManager(juce::AudioProcessorValueTreeState& apvts);

    void savePreset(const juce::String& name);
    bool loadPreset(const juce::File& file);
    void deletePreset(const juce::File& file);

    juce::Array<juce::File> getPresetList() const;
    juce::String getCurrentPresetName() const;
    void setCurrentPresetName(const juce::String& name);

    static juce::File getPresetsFolder();

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::String currentName { "— init —" };
};
```

- [ ] **Step 2: Create PresetManager.cpp**

```cpp
// source/ui/PresetManager.cpp
#include "PresetManager.h"

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& a) : apvts(a) {}

juce::File PresetManager::getPresetsFolder()
{
    auto folder = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                      .getChildFile("Granulator/Presets");
    folder.createDirectory();
    return folder;
}

void PresetManager::savePreset(const juce::String& name)
{
    auto xml = apvts.state.createXml();
    if (!xml) return;
    auto file = getPresetsFolder().getChildFile(name + ".granpreset");
    xml->writeTo(file);
    currentName = name;
}

bool PresetManager::loadPreset(const juce::File& file)
{
    if (!file.existsAsFile()) return false;
    auto xml = juce::XmlDocument::parse(file);
    if (!xml) return false;
    auto tree = juce::ValueTree::fromXml(*xml);
    if (!tree.isValid()) return false;
    apvts.state.copyPropertiesAndChildrenFrom(tree, nullptr);
    currentName = file.getFileNameWithoutExtension();
    return true;
}

void PresetManager::deletePreset(const juce::File& file)
{
    file.deleteFile();
    if (file.getFileNameWithoutExtension() == currentName)
        currentName = "— init —";
}

juce::Array<juce::File> PresetManager::getPresetList() const
{
    juce::Array<juce::File> results;
    getPresetsFolder().findChildFiles(results, juce::File::findFiles, false, "*.granpreset");
    results.sort();
    return results;
}

juce::String PresetManager::getCurrentPresetName() const { return currentName; }
void PresetManager::setCurrentPresetName(const juce::String& n) { currentName = n; }
```

- [ ] **Step 3: Build**
```bash
cmake --build build --config Release
```
Expected: compiles clean (PresetManager not yet referenced by anything).

- [ ] **Step 4: Commit**
```bash
git add source/ui/PresetManager.h source/ui/PresetManager.cpp
git commit -m "feat: add PresetManager for file-based preset I/O"
```

---

## Task 6: PresetBar

**Files:**
- Create: `source/ui/components/PresetBar.h`
- Create: `source/ui/components/PresetBar.cpp`

- [ ] **Step 1: Create PresetBar.h**

```cpp
// source/ui/components/PresetBar.h
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../PresetManager.h"

class PresetBar : public juce::Component
{
public:
    explicit PresetBar(PresetManager& pm);

    void resized() override;
    void paint(juce::Graphics& g) override;

    void updateDisplay();   // call after external preset change (e.g. randomize)

private:
    void showBrowser();
    void stepPreset(int direction);  // -1 = prev, +1 = next
    void trySave();

    PresetManager& presetManager;

    juce::TextButton prevButton  { "<" };
    juce::TextButton nextButton  { ">" };
    juce::TextButton saveButton  { "SAVE" };
    juce::Label      nameLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBar)
};
```

- [ ] **Step 2: Create PresetBar.cpp**

```cpp
// source/ui/components/PresetBar.cpp
#include "PresetBar.h"
#include "../GranularLookAndFeel.h"

PresetBar::PresetBar(PresetManager& pm) : presetManager(pm)
{
    nameLabel.setText(pm.getCurrentPresetName(), juce::dontSendNotification);
    nameLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    nameLabel.setColour(juce::Label::textColourId, GranularLookAndFeel::textPrimary);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    nameLabel.addMouseListener(this, false);

    prevButton.onClick = [this] { stepPreset(-1); };
    nextButton.onClick = [this] { stepPreset(1); };
    saveButton.onClick = [this] { trySave(); };
    nameLabel.addListener(nullptr);  // click handled via mouseDown below

    addAndMakeVisible(prevButton);
    addAndMakeVisible(nameLabel);
    addAndMakeVisible(nextButton);
    addAndMakeVisible(saveButton);
}

void PresetBar::resized()
{
    auto area = getLocalBounds().reduced(0, 4);
    prevButton.setBounds(area.removeFromLeft(24));
    area.removeFromLeft(4);
    saveButton.setBounds(area.removeFromRight(52));
    area.removeFromRight(4);
    nextButton.setBounds(area.removeFromRight(24));
    area.removeFromRight(4);
    nameLabel.setBounds(area);
}

void PresetBar::paint(juce::Graphics& g)
{
    g.setColour(GranularLookAndFeel::backgroundMid);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 2.0f);
    g.setColour(GranularLookAndFeel::divider);
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 2.0f, 1.0f);
}

void PresetBar::updateDisplay()
{
    nameLabel.setText(presetManager.getCurrentPresetName(), juce::dontSendNotification);
}

void PresetBar::showBrowser()
{
    auto presets = presetManager.getPresetList();

    juce::PopupMenu menu;
    if (presets.isEmpty())
    {
        menu.addItem(1, "(no presets saved)", false);
    }
    else
    {
        int id = 100;
        for (auto& f : presets)
            menu.addItem(id++, f.getFileNameWithoutExtension());
    }

    menu.addSeparator();
    menu.addItem(1, "Save Current...");

    auto opts = juce::PopupMenu::Options().withTargetComponent(nameLabel);
    menu.showMenuAsync(opts, [this, presets](int result)
    {
        if (result == 1)
        {
            trySave();
        }
        else if (result >= 100 && result < 100 + presets.size())
        {
            presetManager.loadPreset(presets[result - 100]);
            updateDisplay();
        }
    });
}

void PresetBar::stepPreset(int direction)
{
    auto presets = presetManager.getPresetList();
    if (presets.isEmpty()) return;

    int current = -1;
    auto currentName = presetManager.getCurrentPresetName();
    for (int i = 0; i < presets.size(); ++i)
        if (presets[i].getFileNameWithoutExtension() == currentName)
            { current = i; break; }

    int next = (current + direction + presets.size()) % presets.size();
    presetManager.loadPreset(presets[next]);
    updateDisplay();
}

void PresetBar::trySave()
{
    auto currentName = presetManager.getCurrentPresetName();
    bool isInit = (currentName == "— init —");

    if (!isInit)
    {
        presetManager.savePreset(currentName);
        updateDisplay();
        return;
    }

    // Ask for a name
    auto dialog = std::make_shared<juce::AlertWindow>("Save Preset",
        "Enter a name for this preset:", juce::MessageBoxIconType::NoIcon);
    dialog->addTextEditor("name", "", "Preset name:");
    dialog->addButton("Save", 1);
    dialog->addButton("Cancel", 0);
    dialog->enterModalState(true, juce::ModalCallbackFunction::create(
        [this, dialog](int result)
        {
            if (result == 1)
            {
                auto name = dialog->getTextEditorContents("name").trim();
                if (name.isNotEmpty())
                {
                    presetManager.savePreset(name);
                    updateDisplay();
                }
            }
        }), false);
}

void PresetBar::mouseDown(const juce::MouseEvent& e)
{
    if (e.eventComponent == &nameLabel)
        showBrowser();
}
```

- [ ] **Step 3: Build**
```bash
cmake --build build --config Release
```

- [ ] **Step 4: Commit**
```bash
git add source/ui/components/PresetBar.h source/ui/components/PresetBar.cpp
git commit -m "feat: add PresetBar component with popup browser and save"
```

---

## Task 7: LFORoutingPanel

Owns LFO 1–4 parameter attachments, displays live routing from ModMatrix, plus output knobs (VOL, WET).

**Files:**
- Create: `source/ui/LFORoutingPanel.h`
- Create: `source/ui/LFORoutingPanel.cpp`

- [ ] **Step 1: Create LFORoutingPanel.h**

```cpp
// source/ui/LFORoutingPanel.h
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "components/KnobWithLabel.h"
#include "../modulation/ModMatrix.h"
#include "../dsp/GranularParams.h"

class LFORoutingPanel : public juce::Component, private juce::Timer
{
public:
    LFORoutingPanel(juce::AudioProcessorValueTreeState& apvts, ModMatrix& modMatrix);
    ~LFORoutingPanel() override;

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    void timerCallback() override;
    void paintLFOStrip(juce::Graphics& g, int lfoIndex, juce::Rectangle<int> bounds);
    static juce::String targetName(ModTarget t);

    ModMatrix& modMatrix;

    // Per-LFO controls (4 LFOs)
    struct LFOStrip {
        KnobWithLabel rateKnob, depthKnob, phaseKnob;
        juce::ComboBox shapeBox;
        juce::Label shapeLabel;
        juce::AudioProcessorValueTreeState::SliderAttachment rateAtt, depthAtt, phaseAtt;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> shapeAtt;

        LFOStrip(juce::AudioProcessorValueTreeState& apvts, int n)
            : rateAtt  (apvts, "lfo" + juce::String(n) + "_rate_hz",   rateKnob.getSlider()),
              depthAtt (apvts, "lfo" + juce::String(n) + "_depth",     depthKnob.getSlider()),
              phaseAtt (apvts, "lfo" + juce::String(n) + "_phase_deg", phaseKnob.getSlider())
        {
            rateKnob.setLabel("Rate");
            depthKnob.setLabel("Depth");
            phaseKnob.setLabel("Phase");
            shapeLabel.setText("Shape", juce::dontSendNotification);
            shapeBox.addItemList({"Sine","Triangle","Saw Up","Saw Down","Square","S&H","Smth Rnd"}, 1);
            shapeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                apvts, "lfo" + juce::String(n) + "_shape", shapeBox);
        }
    };

    std::unique_ptr<LFOStrip> strips[4];

    // Output knobs
    KnobWithLabel masterVolKnob, dryWetKnob;
    juce::Label   masterVolLabel, dryWetLabel;
    juce::AudioProcessorValueTreeState::SliderAttachment masterVolAtt, dryWetAtt;

    // Cached routing display (updated by timer)
    struct RoutingEntry { juce::String targetStr; float depth; };
    std::array<juce::Array<RoutingEntry>, 4> cachedRoutings;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LFORoutingPanel)
};
```

- [ ] **Step 2: Create LFORoutingPanel.cpp**

```cpp
// source/ui/LFORoutingPanel.cpp
#include "LFORoutingPanel.h"
#include "GranularLookAndFeel.h"

LFORoutingPanel::LFORoutingPanel(juce::AudioProcessorValueTreeState& apvts, ModMatrix& mm)
    : modMatrix(mm),
      masterVolAtt(apvts, "master_volume_db", masterVolKnob.getSlider()),
      dryWetAtt   (apvts, "dry_wet",          dryWetKnob.getSlider())
{
    for (int i = 0; i < 4; ++i)
    {
        strips[i] = std::make_unique<LFOStrip>(apvts, i + 1);
        addAndMakeVisible(strips[i]->rateKnob);
        addAndMakeVisible(strips[i]->depthKnob);
        addAndMakeVisible(strips[i]->phaseKnob);
        addAndMakeVisible(strips[i]->shapeBox);
        addAndMakeVisible(strips[i]->shapeLabel);
    }

    masterVolKnob.setLabel("Vol");
    dryWetKnob.setLabel("Wet");
    addAndMakeVisible(masterVolKnob);
    addAndMakeVisible(dryWetKnob);

    startTimerHz(10);
}

LFORoutingPanel::~LFORoutingPanel() { stopTimer(); }

void LFORoutingPanel::timerCallback()
{
    // Rebuild routing display from mod matrix
    for (int lfo = 0; lfo < 4; ++lfo)
    {
        cachedRoutings[lfo].clear();
        auto src = static_cast<ModSource>(lfo); // LFO1..LFO4 = 0..3
        for (int s = 0; s < ModMatrix::MAX_SLOTS; ++s)
        {
            auto slot = modMatrix.getSlot(s);
            if (slot.active && slot.source == src)
                cachedRoutings[lfo].add({ targetName(slot.target), slot.depth });
        }
    }
    repaint();
}

void LFORoutingPanel::resized()
{
    const int stripH = (getHeight() - 80) / 4;  // 80px for output section
    const int kw = 54, kh = 66, gap = 6, pad = 6;

    for (int i = 0; i < 4; ++i)
    {
        int y = i * stripH + pad;
        int x = pad;
        strips[i]->rateKnob.setBounds (x, y, kw, kh);  x += kw + gap;
        strips[i]->depthKnob.setBounds(x, y, kw, kh);  x += kw + gap;
        strips[i]->phaseKnob.setBounds(x, y, kw, kh);
        strips[i]->shapeLabel.setBounds(pad, y + kh + 2, 60, 12);
        strips[i]->shapeBox.setBounds  (pad, y + kh + 15, getWidth() - pad*2, 18);
        // routing list is painted in paint()
    }

    // Output knobs at bottom
    int outY = getHeight() - 74;
    masterVolKnob.setBounds(pad,               outY, kw, kh);
    dryWetKnob.setBounds   (pad + kw + gap,    outY, kw, kh);
}

void LFORoutingPanel::paint(juce::Graphics& g)
{
    g.fillAll(GranularLookAndFeel::panelDark);

    // Left border
    g.setColour(GranularLookAndFeel::divider);
    g.drawVerticalLine(0, 0.0f, static_cast<float>(getHeight()));

    const int stripH = (getHeight() - 80) / 4;
    const int kw = 54, kh = 66, gap = 6, pad = 6;

    for (int i = 0; i < 4; ++i)
    {
        int stripY = i * stripH;

        // Strip separator
        if (i > 0)
        {
            g.setColour(GranularLookAndFeel::divider);
            g.drawHorizontalLine(stripY, 0.0f, static_cast<float>(getWidth()));
        }

        // LFO title
        g.setColour(GranularLookAndFeel::accent);
        g.setFont(juce::Font(9.0f, juce::Font::bold));
        g.drawText("LFO " + juce::String(i + 1),
                   pad, stripY + 2, getWidth() - pad*2, 12,
                   juce::Justification::left);

        // Routing list — drawn below the shape combo
        int routingY = stripY + pad + kh + 15 + 20 + 4;
        const auto& routings = cachedRoutings[i];

        if (routings.isEmpty())
        {
            g.setColour(GranularLookAndFeel::textMuted);
            g.setFont(juce::Font(8.5f));
            g.drawText("— unpatched —", pad, routingY, getWidth() - pad*2, 12,
                       juce::Justification::left);
        }
        else
        {
            for (auto& r : routings)
            {
                g.setColour(GranularLookAndFeel::accent);
                g.fillEllipse(static_cast<float>(pad), static_cast<float>(routingY + 3), 4.0f, 4.0f);

                g.setColour(GranularLookAndFeel::textSecondary);
                g.setFont(juce::Font(8.5f));
                g.drawText("→ " + r.targetStr, pad + 8, routingY, 90, 12,
                           juce::Justification::left);

                g.setColour(GranularLookAndFeel::textDim);
                g.drawText(juce::String(r.depth, 2), getWidth() - pad - 32, routingY, 30, 12,
                           juce::Justification::right);

                routingY += 14;
            }
        }
    }

    // Output section separator
    g.setColour(GranularLookAndFeel::divider);
    g.drawHorizontalLine(getHeight() - 80, 0.0f, static_cast<float>(getWidth()));

    g.setColour(GranularLookAndFeel::textSecondary);
    g.setFont(juce::Font(8.0f, juce::Font::bold));
    g.drawText("OUTPUT", pad, getHeight() - 78, 60, 12, juce::Justification::left);
}

juce::String LFORoutingPanel::targetName(ModTarget t)
{
    switch (t)
    {
        case ModTarget::GrainSize:        return "Grain Size";
        case ModTarget::GrainDensity:     return "Density";
        case ModTarget::GrainPitch:       return "Grain Pitch";
        case ModTarget::GrainAmplitude:   return "Grain Amp";
        case ModTarget::GrainPan:         return "Grain Pan";
        case ModTarget::Position:         return "Position";
        case ModTarget::Spray:            return "Spray";
        case ModTarget::ScanRate:         return "Scan Rate";
        case ModTarget::FilterCutoff:     return "Cutoff";
        case ModTarget::FilterResonance:  return "Resonance";
        case ModTarget::PitchShift:       return "Pitch";
        case ModTarget::Detune:           return "Detune";
        case ModTarget::PlaybackRate:     return "Pbk Rate";
        case ModTarget::Amplitude:        return "Amplitude";
        case ModTarget::MasterVolume:     return "Master Vol";
        default:                          return "Unknown";
    }
}
```

- [ ] **Step 3: Build**
```bash
cmake --build build --config Release
```

- [ ] **Step 4: Commit**
```bash
git add source/ui/LFORoutingPanel.h source/ui/LFORoutingPanel.cpp
git commit -m "feat: add LFORoutingPanel with live mod routing display"
```

---

## Task 8: Rewrite PluginEditor — Flat Column Layout + Logo Randomize

**Files:**
- Modify: `source/PluginEditor.h`
- Modify: `source/PluginEditor.cpp`

- [ ] **Step 1: Rewrite PluginEditor.h**

```cpp
// source/PluginEditor.h
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "ui/GranularLookAndFeel.h"
#include "ui/PresetManager.h"
#include "ui/LFORoutingPanel.h"
#include "ui/components/PresetBar.h"
#include "ui/sections/GrainCoreSection.h"
#include "ui/sections/PlayheadSection.h"
#include "ui/sections/PitchSection.h"
#include "ui/sections/AmplitudeSection.h"
#include "ui/sections/SpatialSection.h"
#include "ui/sections/FilterSection.h"
#include "ui/sections/BufferSection.h"
#include "ui/sections/EffectsSection.h"
#include "ui/sections/RecordSection.h"

class PluginEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PluginEditor(PluginProcessor&);
    ~PluginEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    void randomizeAllParameters();
    void paintColumnDividers(juce::Graphics&);
    void paintSectionLabel(juce::Graphics& g, const juce::String& text, juce::Rectangle<int> bounds);

    PluginProcessor& processor;
    GranularLookAndFeel laf;

    PresetManager presetManager;

    // Header
    juce::Label  titleLabel;
    juce::Label  grainCountLabel;
    PresetBar    presetBar;

    // Logo flash animation
    float logoFlashAlpha = 1.0f;
    bool  flashActive    = false;
    int   flashTicks     = 0;

    // Column 1: Grain + Playhead
    GrainCoreSection   grainSection;
    PlayheadSection    playheadSection;

    // Column 2: Pitch + Amplitude
    PitchSection       pitchSection;
    AmplitudeSection   ampSection;

    // Column 3: Space + Filter + Buffer + Record
    SpatialSection     spatialSection;
    FilterSection      filterSection;
    BufferSection      bufferSection;
    RecordSection      recordSection;

    // Column 4: FX
    EffectsSection     fxSection;

    // Column 5: LFO routing + output
    LFORoutingPanel    lfoPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
```

- [ ] **Step 2: Rewrite PluginEditor.cpp**

```cpp
// source/PluginEditor.cpp
#include "PluginEditor.h"

// Column geometry (all sections use the kColW / kColGap constants from GranularLookAndFeel.h)
static int colX(int col) { return kEdgePad + col * (kColW + kColGap); }

PluginEditor::PluginEditor(PluginProcessor& p)
    : AudioProcessorEditor(&p),
      processor(p),
      presetManager    (p.getAPVTS()),
      presetBar        (presetManager),
      grainSection     (p.getAPVTS()),
      playheadSection  (p.getAPVTS()),
      pitchSection     (p.getAPVTS()),
      ampSection       (p.getAPVTS()),
      spatialSection   (p.getAPVTS()),
      filterSection    (p.getAPVTS()),
      bufferSection    (p.getAPVTS()),
      recordSection    (p),
      fxSection        (p.getAPVTS()),
      lfoPanel         (p.getAPVTS(), p.getModMatrix())
{
    setLookAndFeel(&laf);
    setSize(kWindowW, kWindowH);

    // Title label — clickable for randomize
    titleLabel.setText("GRANULATOR", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, GranularLookAndFeel::accent);
    titleLabel.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    titleLabel.addMouseListener(this, false);
    addAndMakeVisible(titleLabel);

    grainCountLabel.setFont(juce::Font(9.0f));
    grainCountLabel.setColour(juce::Label::textColourId, GranularLookAndFeel::textDim);
    grainCountLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(grainCountLabel);

    addAndMakeVisible(presetBar);

    // Col 1
    addAndMakeVisible(grainSection);
    addAndMakeVisible(playheadSection);

    // Col 2
    addAndMakeVisible(pitchSection);
    addAndMakeVisible(ampSection);

    // Col 3
    addAndMakeVisible(spatialSection);
    addAndMakeVisible(filterSection);
    addAndMakeVisible(bufferSection);
    addAndMakeVisible(recordSection);

    // Col 4
    addAndMakeVisible(fxSection);

    // Col 5
    addAndMakeVisible(lfoPanel);

    // Wire buffer file load to waveform in BufferSection
    bufferSection.onLoadFile = [&](const juce::File& file) {
        p.getGrainBuffer().loadFile(file, p.getFormatManager());
        bufferSection.refreshWaveform(file);  // BufferSection owns WaveformDisplay now
    };

    startTimerHz(10);
}

PluginEditor::~PluginEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void PluginEditor::timerCallback()
{
    const int n = processor.getSynth().getTotalActiveGrains();
    grainCountLabel.setText(juce::String(n) + " grains", juce::dontSendNotification);

    // Flash tick
    if (flashActive)
    {
        ++flashTicks;
        const float progress = juce::jmin(1.0f, flashTicks / 6.0f);  // 6 ticks @ 10Hz ≈ 600ms
        // Triangle wave: dim to half-way then back to full
        logoFlashAlpha = (progress < 0.5f)
            ? juce::jmap(progress, 0.0f, 0.5f, 1.0f, 0.25f)
            : juce::jmap(progress, 0.5f, 1.0f, 0.25f, 1.0f);

        if (flashTicks >= 6)
        {
            flashActive = false;
            logoFlashAlpha = 1.0f;
        }
        titleLabel.setColour(juce::Label::textColourId,
            GranularLookAndFeel::accent.withAlpha(logoFlashAlpha));
        titleLabel.repaint();
    }
}

void PluginEditor::mouseDown(const juce::MouseEvent& e)
{
    if (e.eventComponent == &titleLabel)
    {
        randomizeAllParameters();
        flashActive = true;
        flashTicks  = 0;
    }
}

void PluginEditor::randomizeAllParameters()
{
    static const juce::StringArray kExcluded {
        "record_mode", "dc_filter", "anti_aliasing"
    };

    juce::Random rng;
    auto& apvts = processor.getAPVTS();

    for (auto* param : processor.getParameters())
    {
        auto* withID = dynamic_cast<juce::AudioProcessorParameterWithID*>(param);
        if (!withID) continue;

        const juce::String pid = withID->paramID;
        if (kExcluded.contains(pid)) continue;

        float val = rng.nextFloat();  // 0..1 normalised

        if (pid == "grain_probability")
            val = juce::jmap(val, 0.2f, 1.0f);  // avoid full silence

        param->setValueNotifyingHost(val);
    }

    presetManager.setCurrentPresetName("— init —");
    presetBar.updateDisplay();
}

void PluginEditor::paint(juce::Graphics& g)
{
    g.fillAll(GranularLookAndFeel::backgroundDark);

    // Header bar
    g.setColour(GranularLookAndFeel::backgroundMid);
    g.fillRect(0, 0, getWidth(), kHeaderH);

    // Header bottom border
    g.setColour(GranularLookAndFeel::divider);
    g.drawHorizontalLine(kHeaderH, 0.0f, static_cast<float>(getWidth()));

    // Column dividers
    for (int i = 1; i < 5; ++i)
    {
        int x = colX(i) - kColGap / 2;
        g.setColour(GranularLookAndFeel::divider);
        g.drawVerticalLine(x, static_cast<float>(kHeaderH), static_cast<float>(getHeight()));
    }
}

void PluginEditor::resized()
{
    // Header
    titleLabel.setBounds    (kEdgePad, 0, 160, kHeaderH);
    presetBar.setBounds     (180, 4, 600, kHeaderH - 8);
    grainCountLabel.setBounds(getWidth() - 100, 0, 92, kHeaderH);

    const int bodyY = kHeaderH;
    const int bodyH = getHeight() - kHeaderH;

    // Col 1: Grain + Playhead
    // Divide body height: grain ~55%, playhead ~45%
    const int grainH    = (bodyH * 55) / 100;
    const int playhH    = bodyH - grainH;
    grainSection.setBounds   (colX(0), bodyY,          kColW, grainH);
    playheadSection.setBounds(colX(0), bodyY + grainH, kColW, playhH);

    // Col 2: Pitch + Amplitude
    const int pitchH = (bodyH * 45) / 100;
    const int ampH   = bodyH - pitchH;
    pitchSection.setBounds(colX(1), bodyY,           kColW, pitchH);
    ampSection.setBounds  (colX(1), bodyY + pitchH,  kColW, ampH);

    // Col 3: Space + Filter + Buffer + Record
    const int spaceH  = (bodyH * 20) / 100;
    const int filterH = (bodyH * 22) / 100;
    const int bufferH = (bodyH * 45) / 100;
    const int recordH = bodyH - spaceH - filterH - bufferH;
    spatialSection.setBounds(colX(2), bodyY,                              kColW, spaceH);
    filterSection.setBounds (colX(2), bodyY + spaceH,                    kColW, filterH);
    bufferSection.setBounds (colX(2), bodyY + spaceH + filterH,          kColW, bufferH);
    recordSection.setBounds (colX(2), bodyY + spaceH + filterH + bufferH, kColW, recordH);

    // Col 4: FX (full height)
    fxSection.setBounds(colX(3), bodyY, kColW, bodyH);

    // Col 5: LFO panel (full height, uses remaining width to edge)
    lfoPanel.setBounds(colX(4), bodyY, getWidth() - colX(4), bodyH);
}
```

- [ ] **Step 3: Fix BufferSection — add refreshWaveform method**

`BufferSection` currently sends `waveformDisplay` load via the editor. In the new layout, `BufferSection` owns `WaveformDisplay` directly (since it lives in Col 3). If `WaveformDisplay` is already a member of `BufferSection.h`, expose a `refreshWaveform(const juce::File&)` method:

In `source/ui/sections/BufferSection.h`, add:
```cpp
    void refreshWaveform(const juce::File& f);
```

In `source/ui/sections/BufferSection.cpp`, add:
```cpp
void BufferSection::refreshWaveform(const juce::File& f)
{
    waveformDisplay.loadFile(f);
}
```

If `WaveformDisplay` is not already in `BufferSection`, move it there from the editor: add it as a member, add `addAndMakeVisible(waveformDisplay)` in the constructor, and position it in `resized()` as per Task 4 Step 6.

- [ ] **Step 4: Build**
```bash
cmake --build build --config Release
```
Expected: clean build. Common errors to fix:
- Name mismatch on section members (check actual .h files for exact names)
- `AudioProcessorParameterWithID` not found → add `#include <juce_audio_processors/juce_audio_processors.h>` if missing
- `bufferSection.refreshWaveform` missing → ensure step 3 was applied

- [ ] **Step 5: Manual smoke test**

Load the VST3 in a DAW or standalone mode:
- Window opens at 1400×920 ✓
- All 5 columns visible with parameters ✓
- Click GRANULATOR title → all knobs jump to random positions ✓
- Preset bar shows `— init —` after randomize ✓
- LFO routing panel shows `— unpatched —` for all 4 by default ✓
- Save a preset → name appears in preset bar ✓
- Click `<` / `>` to step through presets ✓
- Grain Probability knob in Grain section ✓

- [ ] **Step 6: Commit**
```bash
git add source/PluginEditor.h source/PluginEditor.cpp source/ui/sections/BufferSection.h source/ui/sections/BufferSection.cpp
git commit -m "feat: rewrite PluginEditor as flat column grid with logo randomize and preset bar"
```

---

## Self-Review Notes

- `grain_probability` field added to `GranularParams`, wired through processor → engine ✓
- `randomizeAllParameters` uses `AudioProcessorParameterWithID::paramID` which is available in JUCE 7 for all APVTS-added parameter types ✓
- `kExcluded` covers `record_mode`, `dc_filter`, `anti_aliasing`; `grain_probability` is clamped to [0.2, 1.0] ✓
- `LFORoutingPanel` owns master_volume_db + dry_wet attachments (moved out of PluginEditor) ✓
- `ModSection` removed from `PluginEditor.h` includes and constructor — it is no longer compiled into the editor. The DSP in `ModMatrix.h/cpp` is untouched ✓
- `WaveformDisplay` moves from PluginEditor side panel → BufferSection (Task 8 Step 3 handles this) ✓
- All colour references to old `textSecondary` as a knob-label colour are updated to `textPrimary` / `textSecondary` per new palette ✓
- Section resized() methods verified to fit 3 knobs (76px each) + gaps in 270px column: 3×76 + 2×9 = 246 ≤ 270 ✓
