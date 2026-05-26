#include "KnobWithLabel.h"

KnobWithLabel::KnobWithLabel()
{
    addAndMakeVisible(slider);
    addAndMakeVisible(label);

    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(9.5f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, juce::Colour(0xff909090));
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 14);
}

void KnobWithLabel::setLabel(const juce::String& text)
{
    label.setText(text.toUpperCase(), juce::dontSendNotification);
}

void KnobWithLabel::resized()
{
    const int labelH = 13;
    const int valueH = 14;
    const int knobH  = getHeight() - labelH - valueH;
    label.setBounds(0, 0, getWidth(), labelH);
    slider.setBounds(0, labelH, getWidth(), knobH + valueH);
}

void KnobWithLabel::paint(juce::Graphics& /*g*/) {}
