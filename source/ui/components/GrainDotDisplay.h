#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class GrainDotDisplay : public juce::Component
{
public:
    void setActiveCount(int n) noexcept;
    void paint(juce::Graphics& g) override;

private:
    int activeCount { 0 };

    static constexpr int kMaxGrains = 64;
    static constexpr int kCols      = 8;
    static constexpr int kDot       = 4;
    static constexpr int kGap       = 1;
};
