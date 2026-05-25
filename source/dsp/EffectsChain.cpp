#include "EffectsChain.h"
#include <cmath>

void EffectsChain::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    blockSize  = static_cast<int>(spec.maximumBlockSize);

    // Waveshaper — default tanh
    waveshaper.functionToUse = [](float x) { return std::tanh(x); };
    waveshaper.prepare(spec);

    for (auto& band : eqBands) band.prepare(spec);

    chorus.prepare(spec);
    chorus.setRate(0.5f);
    chorus.setDepth(0.0f);
    chorus.setCentreDelay(7.0f);
    chorus.setFeedback(0.0f);
    chorus.setMix(0.0f);

    // 2 seconds max delay at 96kHz
    const int maxDelaySamples = static_cast<int>(2.0 * spec.sampleRate) + 1;
    delayLine.setMaximumDelayInSamples(maxDelaySamples);
    delayLine.prepare(spec);

    reverb.prepare(spec);
    juce::dsp::Reverb::Parameters rp;
    rp.roomSize = 0.5f; rp.damping = 0.5f; rp.wetLevel = 0.0f; rp.dryLevel = 1.0f;
    reverb.setParameters(rp);

    limiter.prepare(spec);
    limiter.setThreshold(-0.1f);
    limiter.setRelease(100.0f);

    // Default unity EQ
    for (int i = 0; i < 4; ++i)
        updateEQBand(i, 1000.0f * (1 << i), 0.0f, 0.707f);
}

void EffectsChain::updateEQBand(int band, float freq, float gainDb, float q)
{
    const bool isShelf0 = (band == 0);
    const bool isShelf3 = (band == 3);

    juce::dsp::IIR::Coefficients<float>::Ptr coeffs;
    if (isShelf0)
        coeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
            sampleRate, freq, q, juce::Decibels::decibelsToGain(gainDb));
    else if (isShelf3)
        coeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate, freq, q, juce::Decibels::decibelsToGain(gainDb));
    else
        coeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sampleRate, freq, q, juce::Decibels::decibelsToGain(gainDb));

    *eqBands[band].state = *coeffs;
}

void EffectsChain::updateParams(const GranularParams& p)
{
    // Waveshaper
    drive = p.dryWet; // placeholder — actual drive param lives in fx_drive
    // (drive value is read from a separate APVTS raw pointer in PluginProcessor)

    // EQ bands 1-4 updated via PluginProcessor directly

    // Chorus
    chorus.setMix(0.0f); // mix controlled externally

    // Delay
    delayTimeSamples = juce::jlimit(1.0f,
        static_cast<float>(delayLine.getMaximumDelayInSamples() - 1),
        static_cast<float>(p.dryWet)); // placeholder

    // Reverb
    juce::dsp::Reverb::Parameters rp;
    rp.roomSize  = 0.5f;
    rp.damping   = 0.5f;
    rp.wetLevel  = 0.0f;
    rp.dryLevel  = 1.0f;
    rp.width     = 1.0f;
    reverb.setParameters(rp);
}

void EffectsChain::processDelay(juce::AudioBuffer<float>& buf)
{
    if (delayMix <= 0.0f) return;

    const int numSamples = buf.getNumSamples();
    float* L = buf.getWritePointer(0);
    float* R = buf.getWritePointer(1);

    const float delaySamples = delayTimeSamples;
    delayLine.setDelay(delaySamples);

    for (int i = 0; i < numSamples; ++i)
    {
        const float inL = L[i];
        const float inR = R[i];

        const float delL = delayLine.popSample(0);
        const float delR = delayLine.popSample(1);

        if (pingPong)
        {
            delayLine.pushSample(0, inL + delR * delayFb);
            delayLine.pushSample(1, inR + delL * delayFb);
        }
        else
        {
            delayLine.pushSample(0, inL + delL * delayFb);
            delayLine.pushSample(1, inR + delR * delayFb);
        }

        L[i] = inL + delayMix * (delL - inL);
        R[i] = inR + delayMix * (delR - inR);
    }
}

void EffectsChain::process(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);

    // 1. Waveshaper (drive applied in PluginProcessor before this call)
    if (drive > 0.001f)
        waveshaper.process(ctx);

    // 2. EQ
    for (auto& band : eqBands)
        band.process(ctx);

    // 3. Chorus
    chorus.process(ctx);

    // 4. Delay (manual ping-pong routing, bypasses ProcessorChain)
    processDelay(buffer);

    // 5. Reverb
    if (reverbMix > 0.001f)
        reverb.process(ctx);

    // 6. Limiter
    limiter.process(ctx);
}

void EffectsChain::setWaveshaperType(int type) noexcept
{
    if (type == wsType) return;
    wsType = type;
    if (type == 1)
        waveshaper.functionToUse = [](float x) { return x / (1.0f + std::abs(x)); };
    else
        waveshaper.functionToUse = [](float x) { return std::tanh(x); };
}

void EffectsChain::reset()
{
    waveshaper.reset();
    for (auto& b : eqBands) b.reset();
    chorus.reset();
    delayLine.reset();
    reverb.reset();
    limiter.reset();
}
