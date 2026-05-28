#include "ReverbBlobDisplay.h"

ReverbBlobDisplay::ReverbBlobDisplay(juce::AudioProcessorValueTreeState& a) : apvts(a)
{
    startTimerHz(20);
}

ReverbBlobDisplay::~ReverbBlobDisplay() { stopTimer(); }

void ReverbBlobDisplay::timerCallback()
{
    auto get = [&](const char* id) {
        return apvts.getParameterRange(id).convertFrom0to1(apvts.getParameter(id)->getValue());
    };
    const float room = get("fx_reverb_room");
    const float damp = get("fx_reverb_damp");
    const float mix  = get("fx_reverb_mix");

    if (std::abs(room - cachedRoom) > 0.005f ||
        std::abs(damp - cachedDamp) > 0.005f ||
        std::abs(mix  - cachedMix)  > 0.005f)
    {
        cachedRoom = room;
        cachedDamp = damp;
        cachedMix  = mix;
        repaint();
    }
}

void ReverbBlobDisplay::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY();
    const float maxR = std::min(bounds.getWidth(), bounds.getHeight()) * 0.46f;

    g.setColour(juce::Colour(0xff0a0a0a));
    g.fillRect(bounds);
    g.setColour(juce::Colour(0xff282828));
    g.drawRect(bounds, 1.0f);

    if (cachedMix < 0.01f) return;

    // room → spread (how many rings and outer radius)
    // damp → how fast rings decay in alpha (high damp = tight falloff)
    // mix  → overall brightness

    const int   numRings  = 5 + juce::roundToInt(cachedRoom * 4.0f);   // 5–9 rings
    const float outerR    = maxR * (0.3f + cachedRoom * 0.7f);
    const float decayRate = 1.5f + cachedDamp * 4.5f;  // high damp = steep falloff

    for (int i = numRings; i >= 1; --i)
    {
        const float t     = static_cast<float>(i) / numRings;  // 1.0 = outer, 0 = center
        const float r     = outerR * t;
        const float alpha = cachedMix * std::exp(-decayRate * (1.0f - t));

        // Slightly elliptical — wider than tall — to imply spatial spread
        const float rx = r;
        const float ry = r * 0.72f;

        g.setColour(juce::Colour(0xff8ab4d4).withAlpha(alpha * 0.55f));
        g.drawEllipse(cx - rx, cy - ry, rx * 2.0f, ry * 2.0f, 1.0f);
    }

    // Inner core — always slightly visible when mix > 0
    const float coreR  = outerR * 0.12f;
    const float coreMix = cachedMix * 0.8f;
    g.setColour(juce::Colour(0xffa0c8ff).withAlpha(coreMix));
    g.fillEllipse(cx - coreR, cy - coreR * 0.72f, coreR * 2.0f, coreR * 1.44f);
}
