#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <complex>

class EQCurveDisplay : public juce::Component, private juce::Timer
{
public:
    explicit EQCurveDisplay(juce::AudioProcessorValueTreeState& apvts);
    ~EQCurveDisplay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    int hitTest(juce::Point<float> pt) const noexcept;

    void setBandFreqNorm(int band, float norm);
    void setBandGainNorm(int band, float norm);

    float xToFreqNorm(float px) const noexcept;
    float yToGainNorm(float py) const noexcept;

    juce::Point<float> bandDotPosition(int band) const noexcept;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp  (const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;

private:
    void timerCallback() override;
    void rebuildPath();

    static float biquadMagnitudeDB(float freqHz, float sampleRate,
                                   float fcHz, float gainDB, float q) noexcept;

    juce::AudioProcessorValueTreeState& apvts;

    juce::Path curvePath;
    bool       pathDirty { true };

    int  dragBand   { -1 };
    bool isDragging { false };

    static constexpr int   kNumPoints  = 256;
    static constexpr float kSampleRate = 44100.0f;
    static constexpr float kMaxDB      = 18.0f;
    static constexpr float kMinFreq    = 20.0f;
    static constexpr float kMaxFreq    = 20000.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQCurveDisplay)
};
