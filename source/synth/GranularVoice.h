#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "../grain/GranularEngine.h"
#include "../grain/GrainBuffer.h"
#include "../dsp/GranularParams.h"

class GranularSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

class GranularVoice : public juce::SynthesiserVoice
{
public:
    GranularVoice();

    void prepare(const juce::dsp::ProcessSpec& spec, GrainBuffer* sharedBuffer);
    void setParams(const GranularParams& p);

    // juce::SynthesiserVoice
    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newValue) override;
    void controllerMoved(int controllerNumber, int newValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                         int startSample, int numSamples) override;

    int getActiveGrainCount() const noexcept { return engine.getActiveGrainCount(); }
    void collectGrainPositions(juce::Array<float>& out, int bufferLengthSamples) const noexcept
    {
        engine.collectGrainPositions(out, bufferLengthSamples);
    }
    float getVoiceDetuneOffset() const noexcept { return voiceDetuneOffsetCents; }
    void  setVoiceDetuneOffset(float cents) noexcept { voiceDetuneOffsetCents = cents; }

private:
    GranularEngine engine;
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;

    GrainBuffer* buffer = nullptr;
    GranularParams currentParams;

    int   midiNote     = 60;
    float velocity_    = 1.0f;
    float pitchWheelSemitones = 0.0f;
    float voiceDetuneOffsetCents = 0.0f;

    // Glide
    float glideCurrentSt = 0.0f;
    float glideTargetSt  = 0.0f;
    float glideRateSt    = 0.0f;   // semitones per sample

    juce::AudioBuffer<float> scratchBuffer;

    void updateFilter(const GranularParams& p) noexcept;
};
