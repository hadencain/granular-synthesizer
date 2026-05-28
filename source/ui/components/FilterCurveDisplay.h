#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class FilterCurveDisplay : public juce::Component, private juce::Timer
{
public:
    explicit FilterCurveDisplay(juce::AudioProcessorValueTreeState& apvts);
    ~FilterCurveDisplay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void rebuildPath();

    juce::AudioProcessorValueTreeState& apvts;

    float cachedCutoff  = -1.0f;
    float cachedRes     = -1.0f;
    int   cachedType    = -1;

    juce::Path curvePath;

    static constexpr int kNumPoints = 128;
    static constexpr double kSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterCurveDisplay)
};
