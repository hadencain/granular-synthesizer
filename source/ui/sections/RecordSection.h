#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../PluginProcessor.h"

class RecordSection : public juce::Component, private juce::Timer
{
public:
    explicit RecordSection(PluginProcessor& p);
    ~RecordSection() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void toggleRecording();
    juce::String formatTime(double seconds) const;

    PluginProcessor& processor;

    juce::TextButton recordBtn   { "REC" };
    juce::TextButton openFolderBtn { "OPEN FOLDER" };
    juce::Label      statusLabel;
    juce::Label      timeLabel;
    juce::Label      fileLabel;
    juce::Label      headerLabel;

    juce::File outputFolder;
    juce::File currentFile;
};
