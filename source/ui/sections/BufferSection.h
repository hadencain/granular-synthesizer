#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../components/KnobWithLabel.h"

class BufferSection : public juce::Component
{
public:
    explicit BufferSection(juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    KnobWithLabel bufferLength, recordFeedback, inputGain;
    juce::ComboBox  recordModeBox;
    juce::Label     recordModeLabel;

    juce::AudioProcessorValueTreeState::SliderAttachment
        lenAtt, fbAtt, gainAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> recModeAtt;
};
