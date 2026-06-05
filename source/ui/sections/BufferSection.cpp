#include "BufferSection.h"

BufferSection::BufferSection(juce::AudioProcessorValueTreeState& apvts)
    : lenAtt(apvts, "buffer_length_ms", bufferLength.getSlider()),
      fbAtt (apvts, "record_feedback",  recordFeedback.getSlider()),
      gainAtt(apvts, "input_gain_db",   inputGain.getSlider())
{
    bufferLength.setLabel("Buf Length");
    recordFeedback.setLabel("Feedback");
    inputGain.setLabel("Input Gain");
    recordModeLabel.setText("Record Mode", juce::dontSendNotification);

    recordModeBox.addItemList({"Off", "Continuous", "One-Shot", "Gate"}, 1);
    recModeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "record_mode", recordModeBox);

    for (auto* c : { &bufferLength, &recordFeedback, &inputGain })
        addAndMakeVisible(c);
    addAndMakeVisible(recordModeBox);
    addAndMakeVisible(recordModeLabel);
}

void BufferSection::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff555555));
    g.setFont(juce::Font(7.5f, juce::Font::bold));
    g.drawText("BUFFER", 8, 2, getWidth() - 16, 11, juce::Justification::centredLeft);
}

void BufferSection::resized()
{
    const int knobW = 80, knobH = 90, gap = 10;
    int x = gap;
    for (auto* c : { &bufferLength, &recordFeedback, &inputGain })
    { c->setBounds(x, 20, knobW, knobH); x += knobW + gap; }

    recordModeLabel.setBounds(x, 20, 100, 14);
    recordModeBox.setBounds  (x, 36, 100, 22);
}
