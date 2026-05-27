#include "RecordSection.h"

RecordSection::RecordSection(PluginProcessor& p) : processor(p)
{
    outputFolder = juce::File::getSpecialLocation(juce::File::userDesktopDirectory);

    headerLabel.setText("CAPTURE", juce::dontSendNotification);
    headerLabel.setFont(juce::Font(13.0f, juce::Font::bold));
    headerLabel.setColour(juce::Label::textColourId, juce::Colour(0xffd0d0d0));
    addAndMakeVisible(headerLabel);

    statusLabel.setText("IDLE", juce::dontSendNotification);
    statusLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe09020));
    addAndMakeVisible(statusLabel);

    timeLabel.setText("00:00.000", juce::dontSendNotification);
    timeLabel.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 22.0f, juce::Font::plain));
    timeLabel.setColour(juce::Label::textColourId, juce::Colour(0xffd0d0d0));
    addAndMakeVisible(timeLabel);

    fileLabel.setText("—", juce::dontSendNotification);
    fileLabel.setFont(juce::Font(10.0f));
    fileLabel.setColour(juce::Label::textColourId, juce::Colour(0xff909090));
    addAndMakeVisible(fileLabel);

    recordBtn.setButtonText("REC");
    recordBtn.onClick = [this] { toggleRecording(); };
    addAndMakeVisible(recordBtn);

    openFolderBtn.onClick = [this] {
        outputFolder.revealToUser();
    };
    addAndMakeVisible(openFolderBtn);

    startTimerHz(10);
}

RecordSection::~RecordSection()
{
    stopTimer();
    processor.stopRecording();
}

void RecordSection::toggleRecording()
{
    if (processor.getIsRecording())
    {
        processor.stopRecording();
        recordBtn.setButtonText("REC");
        statusLabel.setText("IDLE", juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe09020));
        if (currentFile.existsAsFile())
            fileLabel.setText(currentFile.getFileName(), juce::dontSendNotification);
    }
    else
    {
        const auto timestamp = juce::Time::getCurrentTime()
            .formatted("%Y%m%d_%H%M%S");
        currentFile = outputFolder.getChildFile("granulator_" + timestamp + ".wav");

        processor.startRecording(currentFile);
        recordBtn.setButtonText("STOP");
        statusLabel.setText("RECORDING", juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff4444));
        fileLabel.setText(currentFile.getFileName(), juce::dontSendNotification);
    }
}

juce::String RecordSection::formatTime(double seconds) const
{
    const int mins  = static_cast<int>(seconds) / 60;
    const int secs  = static_cast<int>(seconds) % 60;
    const int ms    = static_cast<int>((seconds - std::floor(seconds)) * 1000.0);
    return juce::String::formatted("%02d:%02d.%03d", mins, secs, ms);
}

void RecordSection::timerCallback()
{
    timeLabel.setText(formatTime(processor.getRecordedLengthSeconds()),
                      juce::dontSendNotification);
}

void RecordSection::paint(juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();

    // Subtle rec indicator dot when recording
    if (processor.getIsRecording())
    {
        const float phase = static_cast<float>(
            juce::Time::getCurrentTime().toMilliseconds() % 1000) / 1000.0f;
        const float alpha = 0.4f + 0.6f * std::abs(std::sin(phase * juce::MathConstants<float>::pi));
        g.setColour(juce::Colour(0xffff3333).withAlpha(alpha));
        g.fillEllipse(b.getRight() - 20.0f, b.getY() + 10.0f, 8.0f, 8.0f);
    }
}

void RecordSection::resized()
{
    const int pad = 20;
    const int btnH = 28;
    const int btnW = 90;

    headerLabel.setBounds(pad, 8, 200, 16);
    timeLabel.setBounds(pad, 28, 260, 30);
    statusLabel.setBounds(pad, 62, 200, 16);
    fileLabel.setBounds(pad, 80, getWidth() - pad * 2, 16);

    recordBtn.setBounds(pad, 100, btnW, btnH);
    openFolderBtn.setBounds(pad + btnW + 12, 100, 120, btnH);
}
