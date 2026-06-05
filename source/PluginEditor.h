#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "dsp/GranularParams.h"
#include "ui/GranularLookAndFeel.h"
#include "ui/components/KnobWithLabel.h"
#include "ui/components/WaveformDisplay.h"
#include "ui/components/GrainDotDisplay.h"
#include "ui/sections/GrainCoreSection.h"
#include "ui/sections/PlayheadSection.h"
#include "ui/sections/PitchSection.h"
#include "ui/sections/AmplitudeSection.h"
#include "ui/sections/SpatialSection.h"
#include "ui/sections/ModSection.h"
#include "ui/sections/FilterSection.h"
#include "ui/sections/BufferSection.h"
#include "ui/sections/EffectsSection.h"

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
    juce::Label      titleLabel;
    GrainDotDisplay  grainDotDisplay;
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
    ModSection         modSection;

    // LFO column footer — master gain
    juce::Slider masterVolSlider { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::Label  masterVolLabel;

    juce::AudioProcessorValueTreeState::SliderAttachment masterVolAtt;

    struct ModKnobBinding
    {
        KnobWithLabel* knob;
        const char*    paramId;  // APVTS param ID for the base value
        ModTarget      target;
        float          scale;    // mod [-1..+1] × scale = native-unit offset
        float          min, max; // native-unit clamp range
    };
    std::vector<ModKnobBinding> modKnobs;

    void updateModDisplay();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
