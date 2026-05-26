#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "GranularVoice.h"
#include "../dsp/GranularParams.h"

class GranularSynth : public juce::Synthesiser
{
public:
    static constexpr int MAX_VOICES = 16;

    GranularSynth();

    void prepare(const juce::dsp::ProcessSpec& spec, GrainBuffer* sharedBuffer);
    void updateParams(const GranularParams& params);
    int  getTotalActiveGrains() const noexcept;
    void collectGrainPositions(juce::Array<float>& out, int bufferLengthSamples) const noexcept;

private:
    void distributeDetuneOffsets(int numVoices, float totalSpreadCents);
};
