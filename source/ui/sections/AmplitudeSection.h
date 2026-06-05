#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../components/KnobWithLabel.h"

class AmplitudeSection : public juce::Component
{
public:
    explicit AmplitudeSection(juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics& g) override;
    void resized() override;

    KnobWithLabel amplitude; // exposed for mod display

private:
    KnobWithLabel ampRandom, velSens, crossfade, attack, decay, sustain, release;

    juce::AudioProcessorValueTreeState::SliderAttachment
        ampAtt, randAtt, velAtt, xfadeAtt, atkAtt, decAtt, susAtt, relAtt;
};
