#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class KnobWithLabel : public juce::Component,
                      private juce::Slider::Listener
{
public:
    KnobWithLabel();
    ~KnobWithLabel() override;

    void setLabel(const juce::String& text);
    juce::Slider& getSlider() { return slider; }

    // Called from editor timer. Sets visual slider position to the modulated native value
    // without touching the APVTS parameter. Suppressed while user is dragging; resumes
    // automatically on the next LFO tick after drag release.
    void pushModulatedValue(double nativeVal);

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    void sliderDragStarted(juce::Slider*) override { userDragging = true; }
    void sliderDragEnded  (juce::Slider*) override { userDragging = false; }
    void sliderValueChanged(juce::Slider*) override {}

    juce::Slider slider { juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow };
    juce::Label  label;
    bool userDragging = false;
};
