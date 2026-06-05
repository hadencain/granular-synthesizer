#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../components/KnobWithLabel.h"
#include "../components/ReverbBlobDisplay.h"
#include "../components/EQCurveDisplay.h"

class EffectsSection : public juce::Component
{
public:
    explicit EffectsSection(juce::AudioProcessorValueTreeState& apvts);
    void resized() override;

private:
    // Waveshaper
    KnobWithLabel drive;
    juce::ComboBox wsTypeBox; juce::Label wsLabel;

    // EQ — interactive curve display; Q hidden sliders for DSP-only access
    EQCurveDisplay eqDisplay;
    std::array<juce::Slider, 4> eqQSliders;

    // Chorus / Delay / Reverb / Limiter
    KnobWithLabel chorusRate, chorusDepth, chorusMix;
    KnobWithLabel delayTime, delayFeedback, delayMix;
    KnobWithLabel reverbRoom, reverbDamp, reverbMix;
    ReverbBlobDisplay reverbBlob;
    KnobWithLabel limiterThresh, limiterRelease;
    juce::ToggleButton pingPongBtn { "Ping-Pong" };

    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> atts;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 4> eqQAtts;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> wsTypeAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   pingPongAtt;
};
