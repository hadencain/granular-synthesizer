#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class ReverbBlobDisplay : public juce::Component, private juce::Timer
{
public:
    explicit ReverbBlobDisplay(juce::AudioProcessorValueTreeState& apvts);
    ~ReverbBlobDisplay() override;

    void paint(juce::Graphics& g) override;

private:
    void timerCallback() override;

    juce::AudioProcessorValueTreeState& apvts;

    float cachedRoom = -1.0f;
    float cachedDamp = -1.0f;
    float cachedMix  = -1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbBlobDisplay)
};
