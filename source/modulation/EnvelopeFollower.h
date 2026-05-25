#pragma once

class EnvelopeFollower
{
public:
    void prepare(double sampleRate) noexcept;
    void setAttackMs(float ms) noexcept;
    void setReleaseMs(float ms) noexcept;

    // Input: audio sample. Returns envelope [0, 1].
    float processSample(float inputSample) noexcept;
    float getCurrentValue() const noexcept { return envelope; }

private:
    double sampleRate = 44100.0;
    float  attackCoeff  = 0.0f;
    float  releaseCoeff = 0.0f;
    float  envelope     = 0.0f;

    float computeCoeff(float ms) const noexcept;
};
