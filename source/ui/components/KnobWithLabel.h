#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class KnobWithLabel : public juce::Component
{
public:
    KnobWithLabel();

    void setLabel(const juce::String& text);
    juce::Slider& getSlider() { return slider; }

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    juce::Slider slider { juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow };
    juce::Label  label;
};
