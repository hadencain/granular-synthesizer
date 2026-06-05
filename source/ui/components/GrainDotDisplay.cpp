#include "GrainDotDisplay.h"

void GrainDotDisplay::setActiveCount(int n) noexcept
{
    const int clamped = juce::jlimit(0, kMaxGrains, n);
    if (clamped != activeCount)
    {
        activeCount = clamped;
        repaint();
    }
}

void GrainDotDisplay::paint(juce::Graphics& g)
{
    constexpr int kRows  = kMaxGrains / kCols;
    constexpr int kPitch = kDot + kGap;

    const float totalW = static_cast<float>(kCols * kPitch - kGap);
    const float totalH = static_cast<float>(kRows * kPitch - kGap);
    const float ox     = (static_cast<float>(getWidth())  - totalW) * 0.5f;
    const float oy     = (static_cast<float>(getHeight()) - totalH) * 0.5f;

    for (int i = 0; i < kMaxGrains; ++i)
    {
        const int   col = i % kCols;
        const int   row = i / kCols;
        const float x   = ox + static_cast<float>(col * kPitch);
        const float y   = oy + static_cast<float>(row * kPitch);

        if (i < activeCount)
        {
            g.setColour(juce::Colour(0xffd0d0d0));
            g.fillEllipse(x, y, static_cast<float>(kDot), static_cast<float>(kDot));
        }
        else
        {
            g.setColour(juce::Colour(0xff2a2a2a));
            g.drawEllipse(x + 0.5f, y + 0.5f,
                          static_cast<float>(kDot) - 1.0f,
                          static_cast<float>(kDot) - 1.0f,
                          1.0f);
        }
    }
}
