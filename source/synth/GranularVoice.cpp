#include "GranularVoice.h"
#include <cmath>

GranularVoice::GranularVoice() = default;

void GranularVoice::prepare(const juce::dsp::ProcessSpec& spec, GrainBuffer* sharedBuffer)
{
    buffer = sharedBuffer;
    engine.prepare(spec.sampleRate, static_cast<int>(spec.maximumBlockSize));
    filter.prepare(spec);
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    filter.setCutoffFrequency(20000.0f);
    filter.setResonance(0.707f);
    scratchBuffer.setSize(2, static_cast<int>(spec.maximumBlockSize));
}

bool GranularVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<GranularSound*>(sound) != nullptr;
}

void GranularVoice::startNote(int midiNoteNumber, float vel,
                               juce::SynthesiserSound*, int)
{
    midiNote  = midiNoteNumber;
    velocity_ = vel;

    const float targetSt = static_cast<float>(midiNoteNumber);
    if (currentParams.glideMs <= 0.0f || !isVoiceActive())
    {
        glideCurrentSt = targetSt;
        glideTargetSt  = targetSt;
        glideRateSt    = 0.0f;
    }
    else
    {
        glideTargetSt = targetSt;
        const float glideSamples = currentParams.glideMs / 1000.0f *
                                   static_cast<float>(getSampleRate());
        glideRateSt = (glideSamples > 0.0f)
            ? std::abs(glideTargetSt - glideCurrentSt) / glideSamples : 0.0f;
    }

    adsr.noteOn();
    engine.resetPlayhead();
}

void GranularVoice::stopNote(float, bool allowTailOff)
{
    if (allowTailOff)
        adsr.noteOff();
    else
    {
        adsr.reset();
        clearCurrentNote();
        engine.reset();
    }
}

void GranularVoice::pitchWheelMoved(int newValue)
{
    // ±2 semitones default range
    pitchWheelSemitones = static_cast<float>(newValue - 8192) / 8192.0f * 2.0f;
}

void GranularVoice::controllerMoved(int, int) {}

void GranularVoice::updateFilter(const GranularParams& p) noexcept
{
    using FilterType = juce::dsp::StateVariableTPTFilterType;
    switch (p.filterType)
    {
        case 0: filter.setType(FilterType::lowpass);  break;
        case 1: filter.setType(FilterType::highpass); break;
        case 2: filter.setType(FilterType::bandpass); break;
        case 3: filter.setType(FilterType::lowpass);  break; // notch not in StateVariableTPTFilter
        default: filter.setType(FilterType::lowpass); break;
    }

    // Keytracking: 100% = cutoff tracks pitch 1:1 from MIDI note 60
    const float keyOffset = (p.midiNote - 60.0f) * p.filterKeytrack *
                            (20000.0f / 127.0f);
    const float modOffset = p.modFilterCutoff * 20000.0f * p.filterLfoDepth;
    const float cutoff = juce::jlimit(20.0f, 20000.0f,
        p.filterCutoffHz + keyOffset + modOffset);
    const float res = juce::jlimit(0.1f, 40.0f,
        p.filterResonance + p.modFilterResonance * 39.9f);

    filter.setCutoffFrequency(cutoff);
    filter.setResonance(res);
}

void GranularVoice::setParams(const GranularParams& p)
{
    currentParams = p;
    currentParams.midiNote  = static_cast<float>(midiNote);
    currentParams.velocity  = velocity_;
    currentParams.voiceDetuneOffset = voiceDetuneOffsetCents;

    adsrParams.attack  = p.adsrAttackMs  / 1000.0f;
    adsrParams.decay   = p.adsrDecayMs   / 1000.0f;
    adsrParams.sustain = p.adsrSustain;
    adsrParams.release = p.adsrReleaseMs / 1000.0f;
    adsr.setParameters(adsrParams);
}

void GranularVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                     int startSample, int numSamples)
{
    if (!adsr.isActive() && !isVoiceActive()) return;
    if (buffer == nullptr) return;

    // --- Glide ---
    if (glideCurrentSt != glideTargetSt && glideRateSt > 0.0f)
    {
        const float diff = glideTargetSt - glideCurrentSt;
        const float step = glideRateSt * static_cast<float>(numSamples);
        if (std::abs(diff) <= step)
            glideCurrentSt = glideTargetSt;
        else
            glideCurrentSt += (diff > 0 ? step : -step);
        currentParams.midiNote = glideCurrentSt;
    }

    updateFilter(currentParams);

    // Render grains into scratch buffer
    scratchBuffer.setSize(2, numSamples, false, false, true);
    scratchBuffer.clear();
    engine.process(scratchBuffer, 0, numSamples, currentParams, *buffer);

    // Apply ADSR envelope (advances envelope state once, applied to both channels)
    adsr.applyEnvelopeToBuffer(scratchBuffer, 0, numSamples);

    // Apply filter (stereo)
    juce::dsp::AudioBlock<float> block(scratchBuffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    filter.process(ctx);

    // Mix into output
    for (int ch = 0; ch < 2; ++ch)
        outputBuffer.addFrom(ch, startSample, scratchBuffer, ch, 0, numSamples);

    // Check if voice has finished
    if (!adsr.isActive())
    {
        clearCurrentNote();
        engine.reset();
    }
}
