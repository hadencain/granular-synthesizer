#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../components/KnobWithLabel.h"

class PlayheadSection : public juce::Component
{
public:
    explicit PlayheadSection(juce::AudioProcessorValueTreeState& apvts);
    void resized() override;

private:
    KnobWithLabel position, spray, scanRate, loopStart, loopEnd, scrub, prob;
    juce::ComboBox  scanShapeBox;
    juce::Label     scanShapeLabel;
    juce::ToggleButton freezeBtn { "Freeze" };

    juce::AudioProcessorValueTreeState::SliderAttachment
        posAtt, sprayAtt, scanRateAtt, loopStartAtt, loopEndAtt, scrubAtt, probAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> scanShapeAtt;
    juce::AudioProcessorValueTreeState::ButtonAttachment   freezeAtt;
};
