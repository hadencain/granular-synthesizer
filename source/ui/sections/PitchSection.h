#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../components/KnobWithLabel.h"

class PitchSection : public juce::Component
{
public:
    explicit PitchSection(juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics& g) override;
    void resized() override;

    KnobWithLabel pitchShift, detune, playbackRate; // exposed for mod display

private:
    KnobWithLabel pitchRandom, formantShift, glide;
    juce::ComboBox transposeBox, quantizeBox;
    juce::Label    transposeLabel, quantizeLabel;

    juce::AudioProcessorValueTreeState::SliderAttachment
        pitchAtt, detuneAtt, randAtt, rateAtt, formantAtt, glideAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> transposeAtt, quantizeAtt;
};
