#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../components/KnobWithLabel.h"
#include "../components/FilterCurveDisplay.h"

class FilterSection : public juce::Component
{
public:
    explicit FilterSection(juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics& g) override;
    void resized() override;

    KnobWithLabel cutoff, resonance; // exposed for mod display

private:
    FilterCurveDisplay curveDisplay;

    KnobWithLabel envDepth, lfoDepth, keytrack;
    juce::ComboBox filterTypeBox;
    juce::Label    filterTypeLabel;

    juce::AudioProcessorValueTreeState::SliderAttachment
        cutoffAtt, resAtt, envDepthAtt, lfoDepthAtt, keytrackAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAtt;
};
