#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../components/KnobWithLabel.h"

class BufferSection : public juce::Component
{
public:
    explicit BufferSection(juce::AudioProcessorValueTreeState& apvts);
    void resized() override;

    std::function<void(const juce::File&)> onLoadFile;

private:
    KnobWithLabel bufferLength, recordFeedback, inputGain;
    juce::ComboBox  recordModeBox;
    juce::Label     recordModeLabel, filePathLabel;
    juce::TextButton loadFileBtn  { "Load File" };
    juce::TextButton clearBufferBtn { "Clear Buffer" };

    juce::AudioProcessorValueTreeState::SliderAttachment
        lenAtt, fbAtt, gainAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> recModeAtt;

    std::unique_ptr<juce::FileChooser> fileChooser;
};
