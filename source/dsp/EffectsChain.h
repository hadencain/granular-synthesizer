#pragma once
#include <juce_dsp/juce_dsp.h>
#include "GranularParams.h"

class EffectsChain
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::AudioBuffer<float>& buffer);
    void updateParams(const GranularParams& p);
    void reset();

    // Accessible from PluginProcessor for param updates
    using EQBand = juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>>;

    using WaveshaperFn = std::function<float(float)>;

    juce::dsp::WaveShaper<float, WaveshaperFn> waveshaper;
    std::array<EQBand, 4>      eqBands;
    juce::dsp::Chorus<float>   chorus;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine;
    juce::dsp::Reverb          reverb;
    juce::dsp::Limiter<float>  limiter;

    double sampleRate  = 44100.0;
    int    blockSize   = 512;
    bool   pingPong    = false;
    float  delayMix    = 0.0f;
    float  delayFb     = 0.0f;
    float  chorusMix   = 0.0f;
    float  reverbMix   = 0.0f;
    float  drive       = 0.0f;
    float  delayTimeSamples = 0.0f;
    int    wsType           = 0;    // 0=Tanh, 1=SoftClip

    void setWaveshaperType(int type) noexcept;
    void updateEQBand(int band, float freq, float gainDb, float q);

private:
    float  delayReadL = 0.0f;
    float  delayReadR = 0.0f;
    void processDelay(juce::AudioBuffer<float>& buf);
};
