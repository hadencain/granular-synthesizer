#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

class GrainBuffer;

struct Grain
{
    bool   active           = false;
    int    age              = 0;
    int    grainSizeSamples = 0;

    double readPosition     = 0.0;  // fractional sample index into GrainBuffer
    double pitchRatio       = 1.0;
    float  amplitude        = 1.0f;
    float  pan              = 0.0f;

    float  panL             = 1.0f; // pre-computed equal-power coefficients
    float  panR             = 1.0f;

    int    envShape         = 0;    // GrainEnvelope::Shape cast to int
    bool   reverse          = false;

    const GrainBuffer* buffer = nullptr; // non-owning

    void reset() noexcept { active = false; age = 0; buffer = nullptr; }

    float normalizedAge() const noexcept
    {
        if (grainSizeSamples <= 0) return 1.0f;
        return static_cast<float>(age) / static_cast<float>(grainSizeSamples);
    }
};
