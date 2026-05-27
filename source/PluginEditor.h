#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "ui/GranularLookAndFeel.h"
#include "ui/components/WaveformDisplay.h"
#include "ui/sections/GrainCoreSection.h"
#include "ui/sections/PlayheadSection.h"
#include "ui/sections/PitchSection.h"
#include "ui/sections/AmplitudeSection.h"
#include "ui/sections/SpatialSection.h"
#include "ui/sections/ModSection.h"
#include "ui/sections/FilterSection.h"
#include "ui/sections/BufferSection.h"
#include "ui/sections/EffectsSection.h"
#include "ui/sections/RecordSection.h"

class PluginEditor : public juce::AudioProcessorEditor,
                     private juce::Timer
{
public:
    explicit PluginEditor(PluginProcessor&);
    ~PluginEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void randomizeAllParams();

    PluginProcessor& processor;
    GranularLookAndFeel laf;

    // Header
    juce::Label      titleLabel, grainCountLabel;
    juce::TextButton startBtn, stopBtn;

    // Waveform strip
    WaveformDisplay waveformDisplay;

    // Section columns
    GrainCoreSection   grainSection;
    PlayheadSection    playheadSection;
    PitchSection       pitchSection;
    AmplitudeSection   ampSection;
    SpatialSection     spatialSection;
    FilterSection      filterSection;
    BufferSection      bufferSection;
    EffectsSection     fxSection;
    RecordSection      recordSection;
    ModSection         modSection;

    // LFO column footer — master out knobs
    juce::Slider masterVolSlider { juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow };
    juce::Label  masterVolLabel;
    juce::Slider dryWetSlider   { juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow };
    juce::Label  dryWetLabel;

    juce::AudioProcessorValueTreeState::SliderAttachment masterVolAtt, dryWetAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
