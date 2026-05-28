#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../../grain/GrainBuffer.h"

class WaveformDisplay : public juce::Component,
                        public juce::FileDragAndDropTarget,
                        private juce::Timer
{
public:
    explicit WaveformDisplay(juce::AudioFormatManager& formatManager);
    ~WaveformDisplay() override;

    void setBuffer(const GrainBuffer* buf) { grainBuffer = buf; }
    void setPlayheadPosition(float normalizedPos) { playheadPos = normalizedPos; repaint(); }
    void loadFile(const juce::File& file);
    void setAPVTS(juce::AudioProcessorValueTreeState* a) { apvts = a; }
    void setGrainPositions(const juce::Array<float>& positions) { grainPositions = positions; }

    // Set by PluginEditor; called when a file is loaded via drop or browse
    std::function<void(const juce::File&)> onLoadFile;

    // FileDragAndDropTarget
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

private:
    void timerCallback() override;
    float toNormalized(int pixelX) const noexcept;
    void setParam(const char* paramID, float normalizedVal);
    void launchFileBrowser();

    juce::AudioThumbnailCache thumbnailCache { 5 };
    juce::AudioThumbnail      thumbnail;
    const GrainBuffer*        grainBuffer = nullptr;
    juce::AudioProcessorValueTreeState* apvts = nullptr;

    float playheadPos = 0.0f;
    juce::Array<float> grainPositions;

    bool  isDragging      = false;
    float dragStartNorm   = 0.0f;
    float dragCurrentNorm = 0.0f;

    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::TextButton replaceBtn { "REPLACE" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};
