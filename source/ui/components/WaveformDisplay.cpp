#include "WaveformDisplay.h"

WaveformDisplay::WaveformDisplay(juce::AudioFormatManager& fm)
    : thumbnail(512, fm, thumbnailCache)
{
    startTimerHz(20);
    setMouseCursor(juce::MouseCursor::CrosshairCursor);
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

float WaveformDisplay::toNormalized(int pixelX) const noexcept
{
    const int w = getWidth();
    if (w <= 0) return 0.0f;
    return juce::jlimit(0.0f, 1.0f, static_cast<float>(pixelX) / static_cast<float>(w));
}

void WaveformDisplay::setParam(const char* paramID, float normalizedVal)
{
    if (apvts == nullptr) return;
    if (auto* param = apvts->getParameter(paramID))
        param->setValueNotifyingHost(normalizedVal);
}

void WaveformDisplay::mouseDown(const juce::MouseEvent& e)
{
    const float norm = toNormalized(e.x);
    isDragging     = false;
    dragStartNorm  = norm;
    dragCurrentNorm = norm;

    // Single click sets playhead position
    setParam("position", norm);
}

void WaveformDisplay::mouseDrag(const juce::MouseEvent& e)
{
    dragCurrentNorm = toNormalized(e.x);
    isDragging = true;

    const float lo = juce::jmin(dragStartNorm, dragCurrentNorm);
    const float hi = juce::jmax(dragStartNorm, dragCurrentNorm);

    setParam("loop_start", lo);
    setParam("loop_end",   hi);

    repaint();
}

void WaveformDisplay::mouseUp(const juce::MouseEvent& /*e*/)
{
    isDragging = false;
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const int  w      = getWidth();
    const int  h      = getHeight();

    // Background
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

    // Loop region overlay — read from APVTS if available, else use drag state
    float loopStart = 0.0f, loopEnd = 0.0f;
    bool  showLoop  = false;

    if (apvts != nullptr)
    {
        if (auto* ps = apvts->getParameter("loop_start"))
            loopStart = ps->getValue();
        if (auto* pe = apvts->getParameter("loop_end"))
            loopEnd = pe->getValue();
        showLoop = (loopEnd > loopStart + 0.005f);
    }

    if (showLoop)
    {
        const float lx = bounds.getX() + loopStart * bounds.getWidth();
        const float lw = (loopEnd - loopStart) * bounds.getWidth();
        g.setColour(juce::Colour(0xaa8b0000));   // dark red, semi-transparent
        g.fillRect(lx, bounds.getY(), lw, bounds.getHeight());
        // Loop boundary lines
        g.setColour(juce::Colour(0xffcc2020));
        g.drawLine(lx,      bounds.getY() + 1.0f, lx,      bounds.getBottom() - 1.0f, 1.0f);
        g.drawLine(lx + lw, bounds.getY() + 1.0f, lx + lw, bounds.getBottom() - 1.0f, 1.0f);
    }

    // Active grain position markers — white vertical lines
    for (float pos : grainPositions)
    {
        const float gx = bounds.getX() + pos * bounds.getWidth();
        g.setColour(juce::Colours::white.withAlpha(0.6f));
        g.drawLine(gx, bounds.getY() + 2.0f, gx, bounds.getBottom() - 2.0f, 1.0f);
    }

    // Playhead line
    if (playheadPos >= 0.0f && playheadPos <= 1.0f)
    {
        const float x = bounds.getX() + playheadPos * bounds.getWidth();
        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.drawLine(x, bounds.getY() + 2.0f, x, bounds.getBottom() - 2.0f, 1.5f);
    }

    juce::ignoreUnused(w, h);
}
