#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include "Grain.h"
#include "GrainBuffer.h"
#include "../dsp/GranularParams.h"
#include <array>

class GranularEngine
{
public:
    static constexpr int MAX_GRAINS = 512;

    void prepare(double sampleRate, int blockSize);
    void reset();

    void process(juce::AudioBuffer<float>& outputBuffer,
                 int startSample, int numSamples,
                 const GranularParams& params,
                 const GrainBuffer& buffer);

    int getActiveGrainCount() const noexcept;

    // Resets playhead position (e.g. on note retrigger)
    void resetPlayhead() noexcept;

private:
    std::array<Grain, MAX_GRAINS> grainPool;
    int nextGrainIndex = 0;

    double schedulerCountdown = 0.0;  // samples until next grain spawn
    double playheadPos        = 0.0;  // current read position in samples
    double scanPhase          = 0.0;  // for Sine scan shape [0, 2pi)
    int    pendulumDir        = 1;    // +1 / -1 for Pendulum mode

    double sampleRate = 44100.0;
    mutable juce::Random rng;

    Grain* allocateGrain() noexcept;
    void   spawnGrain(const GranularParams& p, const GrainBuffer& buf, double bufLenSamples);
    void   advancePlayhead(const GranularParams& p, int numSamples, double bufLenSamples);
    double computeInteronsetSamples(const GranularParams& p) const noexcept;
};
