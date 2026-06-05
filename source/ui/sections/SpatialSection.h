#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../components/KnobWithLabel.h"

class SpatialSection : public juce::Component
{
public:
    explicit SpatialSection(juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics& g) override;
    void resized() override;
    void paintOverChildren(juce::Graphics& g) override;

private:
    KnobWithLabel pan, panRandom, stereoWidth, voiceDetune;
    juce::Slider  voicesSlider { juce::Slider::LinearHorizontal, juce::Slider::NoTextBox };
    juce::Label   voicesLabel;

    juce::AudioProcessorValueTreeState::SliderAttachment
        panAtt, panRandAtt, widthAtt, detuneAtt, voicesAtt;
};
