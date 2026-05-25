#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class GranularLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GranularLookAndFeel();

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override;

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    void drawTabButton(juce::TabBarButton& button, juce::Graphics& g,
                       bool isMouseOver, bool isMouseDown) override;

    void drawTabAreaBehindFrontButton(juce::TabbedButtonBar& bar, juce::Graphics& g,
                                      int w, int h) override;

    // Colour palette
    static const juce::Colour backgroundDark;
    static const juce::Colour backgroundMid;
    static const juce::Colour accent;
    static const juce::Colour textPrimary;
    static const juce::Colour textSecondary;
    static const juce::Colour knobBody;
    static const juce::Colour knobThumb;
};
