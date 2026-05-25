#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "../../grain/GrainBuffer.h"

class WaveformDisplay : public juce::Component, private juce::Timer
{
public:
    explicit WaveformDisplay(juce::AudioFormatManager& formatManager);
    ~WaveformDisplay() override;

    void setBuffer(const GrainBuffer* buf) { grainBuffer = buf; }
    void setPlayheadPosition(float normalizedPos) { playheadPos = normalizedPos; repaint(); }
    void loadFile(const juce::File& file);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;

    juce::AudioThumbnailCache thumbnailCache { 5 };
    juce::AudioThumbnail      thumbnail;
    const GrainBuffer*        grainBuffer = nullptr;
    float                     playheadPos = 0.0f;
};
