#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../components/KnobWithLabel.h"

class GrainCoreSection : public juce::Component
{
public:
    explicit GrainCoreSection(juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics& g) override;
    void resized() override;

    KnobWithLabel grainSize, grainDensity; // exposed for mod display

private:
    KnobWithLabel grainOverlap, interonset, randomizeSize, randomizeDensity;
    juce::ComboBox envelopeBox, directionBox;
    juce::Label    envelopeLabel, directionLabel;

    juce::AudioProcessorValueTreeState::SliderAttachment
        sizeAtt, densityAtt, overlapAtt, interonsetAtt, randSizeAtt, randDensityAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> envAtt, dirAtt;
};
