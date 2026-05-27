#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include "../dsp/GranularParams.h"

class LFO
{
public:
    void prepare(double sampleRate) noexcept;
    void reset() noexcept;

    // Returns current output sample in [-1, +1], advances phase
    float process() noexcept;
    float processBlock(int numSamples) noexcept;

    void setRate(float hz) noexcept     { rateHz = hz; }
    void setShape(LFOShape s) noexcept  { shape = s; }
    void setPhaseOffset(float deg) noexcept;  // 0..360

    float getCurrentValue() const noexcept { return lastValue; }

private:
    double sampleRate = 44100.0;
    float  rateHz     = 1.0f;
    LFOShape shape    = LFOShape::Sine;
    float  phase      = 0.0f;       // [0, 1)
    float  phaseOffset = 0.0f;      // [0, 1)
    float  lastValue  = 0.0f;
    float  shValue    = 0.0f;       // held value for S&H
    juce::Random rng;

    static constexpr float kTwoPi = 6.28318530717958647692f;
};
