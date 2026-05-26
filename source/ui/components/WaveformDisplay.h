#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../../grain/GrainBuffer.h"

class WaveformDisplay : public juce::Component, private juce::Timer
{
public:
    explicit WaveformDisplay(juce::AudioFormatManager& formatManager);
    ~WaveformDisplay() override;

    void setBuffer(const GrainBuffer* buf) { grainBuffer = buf; }
    void setPlayheadPosition(float normalizedPos) { playheadPos = normalizedPos; repaint(); }
    void loadFile(const juce::File& file);

    // Wire in APVTS so mouse interactions can update position/loop_start/loop_end params
    void setAPVTS(juce::AudioProcessorValueTreeState* a) { apvts = a; }

    // Called by PluginEditor's timer to pass current grain positions (normalized 0-1)
    void setGrainPositions(const juce::Array<float>& positions) { grainPositions = positions; }

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

private:
    void timerCallback() override;
    float toNormalized(int pixelX) const noexcept;
    void setParam(const char* paramID, float normalizedVal);

    juce::AudioThumbnailCache thumbnailCache { 5 };
    juce::AudioThumbnail      thumbnail;
    const GrainBuffer*        grainBuffer = nullptr;
    juce::AudioProcessorValueTreeState* apvts = nullptr;

    float playheadPos = 0.0f;
    juce::Array<float> grainPositions;

    // Drag state
    bool  isDragging    = false;
    float dragStartNorm = 0.0f;
    float dragCurrentNorm = 0.0f;
};
