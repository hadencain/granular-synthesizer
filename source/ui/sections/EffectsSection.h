#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../components/KnobWithLabel.h"

class EffectsSection : public juce::Component
{
public:
    explicit EffectsSection(juce::AudioProcessorValueTreeState& apvts);
    void resized() override;

private:
    // Waveshaper
    KnobWithLabel drive;
    juce::ComboBox wsTypeBox; juce::Label wsLabel;

    // EQ (4 bands, each: freq/gain/q)
    struct EQBandKnobs { KnobWithLabel freq, gain, q; };
    std::array<EQBandKnobs, 4> eq;

    // Chorus / Delay / Reverb / Limiter
    KnobWithLabel chorusRate, chorusDepth, chorusMix;
    KnobWithLabel delayTime, delayFeedback, delayMix;
    KnobWithLabel reverbRoom, reverbDamp, reverbMix;
    KnobWithLabel limiterThresh, limiterRelease;
    juce::ToggleButton pingPongBtn { "Ping-Pong" };

    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> atts;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> wsTypeAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   pingPongAtt;
};
