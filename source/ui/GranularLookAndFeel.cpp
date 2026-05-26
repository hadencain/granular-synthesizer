#include "GranularLookAndFeel.h"

const juce::Colour GranularLookAndFeel::backgroundDark = juce::Colour(0xff0c0c0c);
const juce::Colour GranularLookAndFeel::backgroundMid  = juce::Colour(0xff141414);
const juce::Colour GranularLookAndFeel::accent         = juce::Colour(0xffffffff);
const juce::Colour GranularLookAndFeel::textPrimary    = juce::Colour(0xffd0d0d0);
const juce::Colour GranularLookAndFeel::textSecondary  = juce::Colour(0xff909090);
const juce::Colour GranularLookAndFeel::knobBody       = juce::Colour(0xff202020);
const juce::Colour GranularLookAndFeel::knobThumb      = juce::Colour(0xffffffff);

GranularLookAndFeel::GranularLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, backgroundDark);
    setColour(juce::Slider::thumbColourId,               accent);
    setColour(juce::Slider::trackColourId,               accent.withAlpha(0.4f));
    setColour(juce::Slider::backgroundColourId,          knobBody);
    setColour(juce::ComboBox::backgroundColourId,        backgroundMid);
    setColour(juce::ComboBox::textColourId,              textPrimary);
    setColour(juce::ComboBox::outlineColourId,           textSecondary.withAlpha(0.6f));
    setColour(juce::ComboBox::arrowColourId,             textSecondary);
    setColour(juce::Label::textColourId,                 textSecondary);
    setColour(juce::TabbedButtonBar::tabTextColourId,    textSecondary);
    setColour(juce::TabbedButtonBar::frontTextColourId,  accent);
    setColour(juce::TabbedComponent::backgroundColourId, backgroundDark);
    setColour(juce::ToggleButton::textColourId,          textPrimary);
    setColour(juce::ToggleButton::tickColourId,          accent);
    setColour(juce::ToggleButton::tickDisabledColourId,  textSecondary);
    setColour(juce::PopupMenu::backgroundColourId,                    backgroundMid);
    setColour(juce::PopupMenu::textColourId,                          textPrimary);
    setColour(juce::PopupMenu::highlightedBackgroundColourId,         accent.withAlpha(0.1f));
    setColour(juce::PopupMenu::highlightedTextColourId,               accent);
    setColour(juce::TextButton::buttonColourId,   backgroundMid);
    setColour(juce::TextButton::textColourOffId,  textPrimary);
    setColour(juce::TextButton::buttonOnColourId, accent.withAlpha(0.12f));
    setColour(juce::TextButton::textColourOnId,   accent);
    setColour(juce::TableHeaderComponent::backgroundColourId, backgroundMid);
    setColour(juce::TableHeaderComponent::textColourId,       textSecondary);
    setColour(juce::ListBox::backgroundColourId, backgroundDark);
    setColour(juce::ListBox::textColourId,        textPrimary);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxTextColourId,       textPrimary);
}

void GranularLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                            int x, int y, int width, int height,
                                            float sliderPos,
                                            float rotaryStartAngle,
                                            float rotaryEndAngle,
                                            juce::Slider& /*slider*/)
{
    const float radius  = juce::jmin(width / 2.0f, height / 2.0f) - 6.0f;
    const float centreX = x + width  * 0.5f;
    const float centreY = y + height * 0.5f;
    const float angle   = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Background arc
    {
        juce::Path arc;
        arc.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                          rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(knobBody);
        g.strokePath(arc, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
    }

    // Value arc
    {
        juce::Path arc;
        arc.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                          rotaryStartAngle, angle, true);
        g.setColour(accent);
        g.strokePath(arc, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
    }

    // Tick line from inner to outer rim — position indicator
    const float sinA   = std::sin(angle);
    const float cosA   = std::cos(angle);
    const float innerR = radius * 0.32f;
    const float outerR = radius * 0.72f;
    g.setColour(juce::Colour(0xffc89040));   // amber tick
    g.drawLine(centreX + innerR * sinA, centreY - innerR * cosA,
               centreX + outerR * sinA, centreY - outerR * cosA,
               1.5f);
}

void GranularLookAndFeel::drawToggleButton(juce::Graphics& g,
                                            juce::ToggleButton& button,
                                            bool shouldDrawButtonAsHighlighted,
                                            bool /*shouldDrawButtonAsDown*/)
{
    const float w  = static_cast<float>(button.getWidth());
    const float h  = static_cast<float>(button.getHeight());
    const bool  on = button.getToggleState();

    const float top = h * 0.18f;
    const float ht  = h * 0.64f;

    juce::Colour bg = on ? accent.withAlpha(0.1f) : juce::Colours::transparentBlack;
    if (shouldDrawButtonAsHighlighted && !on)
        bg = accent.withAlpha(0.04f);

    g.setColour(bg);
    g.fillRect(2.0f, top, w - 4.0f, ht);

    g.setColour(on ? accent : textSecondary);
    g.drawRect(2.0f, top, w - 4.0f, ht, 1.0f);

    g.setColour(on ? accent : textPrimary);
    g.setFont(juce::Font(h * 0.33f, juce::Font::bold));
    g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred);
}

void GranularLookAndFeel::drawTabButton(juce::TabBarButton& button, juce::Graphics& g,
                                         bool isMouseOver, bool /*isMouseDown*/)
{
    const bool  isActive = button.isFrontTab();
    const auto  bounds   = button.getLocalBounds().toFloat();

    if (isMouseOver && !isActive)
    {
        g.setColour(accent.withAlpha(0.04f));
        g.fillRect(bounds);
    }

    g.setColour(isActive ? accent : (isMouseOver ? accent.withAlpha(0.75f) : textPrimary));
    g.setFont(juce::Font(10.5f, juce::Font::bold));
    g.drawText(button.getButtonText().toUpperCase(),
               bounds.reduced(4.0f, 0.0f),
               juce::Justification::centred);

    if (isActive)
    {
        g.setColour(accent);
        g.fillRect(bounds.getX() + 6.0f,
                   bounds.getBottom() - 2.0f,
                   bounds.getWidth() - 12.0f,
                   2.0f);
    }
}

void GranularLookAndFeel::drawTabAreaBehindFrontButton(juce::TabbedButtonBar& /*bar*/,
                                                        juce::Graphics& g,
                                                        int w, int h)
{
    g.setColour(backgroundMid);
    g.fillRect(0, 0, w, h);
    // Hairline at the bottom of the tab bar
    g.setColour(textSecondary.withAlpha(0.25f));
    g.drawHorizontalLine(h - 1, 0.0f, static_cast<float>(w));
}
