#include "EnvelopeFollower.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

void EnvelopeFollower::prepare(double sr) noexcept
{
    sampleRate = sr;
    setAttackMs(10.0f);
    setReleaseMs(100.0f);
    envelope = 0.0f;
}

float EnvelopeFollower::computeCoeff(float ms) const noexcept
{
    if (ms <= 0.0f) return 0.0f;
    return std::exp(-1.0f / (static_cast<float>(sampleRate) * ms / 1000.0f));
}

void EnvelopeFollower::setAttackMs(float ms) noexcept
{
    attackCoeff = computeCoeff(ms);
}

void EnvelopeFollower::setReleaseMs(float ms) noexcept
{
    releaseCoeff = computeCoeff(ms);
}

float EnvelopeFollower::processSample(float inputSample) noexcept
{
    const float rectified = std::abs(inputSample);
    const float coeff = (rectified > envelope) ? attackCoeff : releaseCoeff;
    envelope = coeff * envelope + (1.0f - coeff) * rectified;
    return envelope;
}
