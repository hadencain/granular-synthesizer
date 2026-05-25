#include "BufferSection.h"

BufferSection::BufferSection(juce::AudioProcessorValueTreeState& apvts)
    : lenAtt    (apvts, "buffer_length_ms",  bufferLength.getSlider()),
      fbAtt     (apvts, "record_feedback",   recordFeedback.getSlider()),
      gainAtt   (apvts, "input_gain_db",     inputGain.getSlider()),
      recModeAtt(apvts, "record_mode",       recordModeBox)
{
    bufferLength.setLabel("Buf Length");
    recordFeedback.setLabel("Feedback");
    inputGain.setLabel("Input Gain");
    recordModeLabel.setText("Record Mode", juce::dontSendNotification);
    filePathLabel.setText("No file loaded", juce::dontSendNotification);

    for (auto* c : { &bufferLength, &recordFeedback, &inputGain })
        addAndMakeVisible(c);
    addAndMakeVisible(recordModeBox);
    addAndMakeVisible(recordModeLabel);
    addAndMakeVisible(filePathLabel);
    addAndMakeVisible(loadFileBtn);
    addAndMakeVisible(clearBufferBtn);

    loadFileBtn.onClick = [this] {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Load Audio File", juce::File{}, "*.wav;*.aif;*.aiff;*.flac;*.mp3");
        fileChooser->launchAsync(juce::FileBrowserComponent::openMode,
            [this](const juce::FileChooser& fc) {
                if (fc.getResults().isEmpty()) return;
                const auto file = fc.getResult();
                filePathLabel.setText(file.getFileName(), juce::dontSendNotification);
                if (onLoadFile) onLoadFile(file);
            });
    };
}

void BufferSection::resized()
{
    const int knobW = 80, knobH = 90, gap = 10;
    int x = gap;
    for (auto* c : { &bufferLength, &recordFeedback, &inputGain })
    { c->setBounds(x, 10, knobW, knobH); x += knobW + gap; }

    recordModeLabel.setBounds(x, 10, 100, 14);
    recordModeBox.setBounds  (x, 26, 100, 22);

    loadFileBtn.setBounds   (gap, 10 + knobH + gap, 100, 24);
    filePathLabel.setBounds (gap + 110, 10 + knobH + gap + 2, 300, 20);
    clearBufferBtn.setBounds(gap, 10 + knobH + gap + 32, 100, 24);
}
