#include "GranularSynth.h"

GranularSynth::GranularSynth()
{
    for (int i = 0; i < MAX_VOICES; ++i)
        addVoice(new GranularVoice());

    addSound(new GranularSound());
    setNoteStealingEnabled(true);
}

void GranularSynth::prepare(const juce::dsp::ProcessSpec& spec, GrainBuffer* sharedBuffer)
{
    setCurrentPlaybackSampleRate(spec.sampleRate);

    for (int i = 0; i < getNumVoices(); ++i)
    {
        if (auto* v = dynamic_cast<GranularVoice*>(getVoice(i)))
            v->prepare(spec, sharedBuffer);
    }
}

void GranularSynth::updateParams(const GranularParams& params)
{
    const int numVoices = juce::jlimit(1, MAX_VOICES, params.voices);
    distributeDetuneOffsets(numVoices, params.voiceDetuneCents);

    for (int i = 0; i < getNumVoices(); ++i)
    {
        if (auto* v = dynamic_cast<GranularVoice*>(getVoice(i)))
        {
            GranularParams vp = params;
            vp.voiceDetuneOffset = v->getVoiceDetuneOffset();
            v->setParams(vp);
        }
    }
}

void GranularSynth::distributeDetuneOffsets(int numVoices, float totalSpreadCents)
{
    for (int i = 0; i < getNumVoices(); ++i)
    {
        if (auto* v = dynamic_cast<GranularVoice*>(getVoice(i)))
        {
            if (numVoices <= 1)
            {
                v->setVoiceDetuneOffset(0.0f);
            }
            else
            {
                // Symmetric distribution: voice 0 gets -spread/2, last gets +spread/2
                const float offset = totalSpreadCents *
                    (static_cast<float>(i) / static_cast<float>(numVoices - 1) - 0.5f);
                v->setVoiceDetuneOffset(offset);
            }
        }
    }
}

int GranularSynth::getTotalActiveGrains() const noexcept
{
    int total = 0;
    for (int i = 0; i < getNumVoices(); ++i)
        if (const auto* v = dynamic_cast<const GranularVoice*>(getVoice(i)))
            total += v->getActiveGrainCount();
    return total;
}

void GranularSynth::collectGrainPositions(juce::Array<float>& out, int bufferLengthSamples) const noexcept
{
    for (int i = 0; i < getNumVoices(); ++i)
        if (const auto* v = dynamic_cast<const GranularVoice*>(getVoice(i)))
            v->collectGrainPositions(out, bufferLengthSamples);
}
