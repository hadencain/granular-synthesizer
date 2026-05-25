#include "WaveformDisplay.h"

WaveformDisplay::WaveformDisplay(juce::AudioFormatManager& fm)
    : thumbnail(512, fm, thumbnailCache)
{
    startTimerHz(20);
}

WaveformDisplay::~WaveformDisplay()
{
    stopTimer();
}

void WaveformDisplay::resized() {}

void WaveformDisplay::loadFile(const juce::File& file)
{
    thumbnail.setSource(new juce::FileInputSource(file));
}

void WaveformDisplay::timerCallback()
{
    repaint();
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    // Background — slightly lighter than panel
    g.setColour(juce::Colour(0xff0e0e0e));
    g.fillRect(bounds);

    // Border
    g.setColour(juce::Colour(0xff282828));
    g.drawRect(bounds, 1.0f);

    if (thumbnail.getTotalLength() > 0.0)
    {
        g.setColour(juce::Colours::white.withAlpha(0.55f));
        thumbnail.drawChannels(g, getLocalBounds().reduced(4),
                               0.0, thumbnail.getTotalLength(), 1.0f);
    }
    else
    {
        g.setColour(juce::Colour(0xff666666));
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText("NO AUDIO", getLocalBounds(), juce::Justification::centred);
    }

    // Playhead line
    if (playheadPos >= 0.0f && playheadPos <= 1.0f)
    {
        const float x = bounds.getX() + playheadPos * bounds.getWidth();
        g.setColour(juce::Colours::white.withAlpha(0.7f));
        g.drawLine(x, bounds.getY() + 2.0f, x, bounds.getBottom() - 2.0f, 1.0f);
    }
}
