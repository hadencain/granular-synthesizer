#include "LFO.h"
#include <cmath>

void LFO::prepare(double sr) noexcept
{
    sampleRate = sr;
    reset();
}

void LFO::reset() noexcept
{
    phase    = phaseOffset;
    lastValue = 0.0f;
    shValue  = 0.0f;
}

void LFO::setPhaseOffset(float degrees) noexcept
{
    phaseOffset = degrees / 360.0f;
}

float LFO::process() noexcept
{
    const float p = phase + phaseOffset;
    const float wrapped = p - std::floor(p); // [0, 1)

    float value = 0.0f;
    switch (shape)
    {
        case LFOShape::Sine:
            value = std::sin(wrapped * kTwoPi);
            break;

        case LFOShape::Triangle:
            value = (wrapped < 0.5f)
                ? (4.0f * wrapped - 1.0f)
                : (3.0f - 4.0f * wrapped);
            break;

        case LFOShape::SawUp:
            value = 2.0f * wrapped - 1.0f;
            break;

        case LFOShape::SawDown:
            value = 1.0f - 2.0f * wrapped;
            break;

        case LFOShape::Square:
            value = (wrapped < 0.5f) ? 1.0f : -1.0f;
            break;

        case LFOShape::SampleHold:
        {
            const float prevPhase = phase;
            // Trigger new sample when phase wraps
            if (wrapped < (rateHz / static_cast<float>(sampleRate)))
                shValue = rng.nextFloat() * 2.0f - 1.0f;
            value = shValue;
            break;
        }

        case LFOShape::SmoothRandom:
            // Brownian-style: small random step per sample, clamped to [-1, +1]
            shValue = juce::jlimit(-1.0f, 1.0f,
                shValue + (rng.nextFloat() * 2.0f - 1.0f) * 0.05f);
            value = shValue;
            break;
    }

    lastValue = value;

    // Advance phase
    const double phaseIncrement = static_cast<double>(rateHz) / sampleRate;
    phase = static_cast<float>(std::fmod(static_cast<double>(phase) + phaseIncrement, 1.0));

    return value;
}

float LFO::processBlock(int numSamples) noexcept
{
    const float p = phase + phaseOffset;
    const float wrapped = p - std::floor(p); // [0, 1)

    float value = 0.0f;
    switch (shape)
    {
        case LFOShape::Sine:
            value = std::sin(wrapped * kTwoPi);
            break;

        case LFOShape::Triangle:
            value = (wrapped < 0.5f)
                ? (4.0f * wrapped - 1.0f)
                : (3.0f - 4.0f * wrapped);
            break;

        case LFOShape::SawUp:
            value = 2.0f * wrapped - 1.0f;
            break;

        case LFOShape::SawDown:
            value = 1.0f - 2.0f * wrapped;
            break;

        case LFOShape::Square:
            value = (wrapped < 0.5f) ? 1.0f : -1.0f;
            break;

        case LFOShape::SampleHold:
            if (wrapped < (rateHz / static_cast<float>(sampleRate) * numSamples))
                shValue = rng.nextFloat() * 2.0f - 1.0f;
            value = shValue;
            break;

        case LFOShape::SmoothRandom:
            shValue = juce::jlimit(-1.0f, 1.0f,
                shValue + (rng.nextFloat() * 2.0f - 1.0f) * 0.05f);
            value = shValue;
            break;
    }

    lastValue = value;

    const double phaseIncrement = static_cast<double>(rateHz) / sampleRate
                                  * static_cast<double>(numSamples);
    phase = static_cast<float>(std::fmod(static_cast<double>(phase) + phaseIncrement, 1.0));

    return value;
}
