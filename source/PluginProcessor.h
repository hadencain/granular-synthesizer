#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_dsp/juce_dsp.h>
#include "grain/GrainBuffer.h"
#include "synth/GranularSynth.h"
#include "modulation/ModMatrix.h"
#include "modulation/LFO.h"
#include "modulation/EnvelopeFollower.h"
#include "dsp/EffectsChain.h"
#include "dsp/GranularParams.h"

class PluginProcessor : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Granulator"; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int  getNumPrograms() override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Accessors for the editor
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    GrainBuffer&   getGrainBuffer()  noexcept { return grainBuffer; }
    ModMatrix&     getModMatrix()    noexcept { return modMatrix; }
    GranularSynth& getSynth()        noexcept { return synth; }
    juce::AudioFormatManager& getFormatManager() noexcept { return formatManager; }

    // Raw param pointers cached at prepareToPlay for zero-overhead audio-thread reads
    struct RawParams
    {
        // Grain core
        std::atomic<float>* grainSizeMs       = nullptr;
        std::atomic<float>* grainDensity      = nullptr;
        std::atomic<float>* grainEnvelope     = nullptr;
        std::atomic<float>* grainOverlap      = nullptr;
        std::atomic<float>* interonsetMs      = nullptr;
        std::atomic<float>* grainDirection    = nullptr;
        std::atomic<float>* randomizeSizeMs   = nullptr;
        std::atomic<float>* randomizeDensity  = nullptr;
        std::atomic<float>* grainProbability  = nullptr;
        // Playhead
        std::atomic<float>* position          = nullptr;
        std::atomic<float>* sprayMs           = nullptr;
        std::atomic<float>* scanRateHz        = nullptr;
        std::atomic<float>* scanShape         = nullptr;
        std::atomic<float>* loopStart         = nullptr;
        std::atomic<float>* loopEnd           = nullptr;
        std::atomic<float>* freeze            = nullptr;
        std::atomic<float>* scrub             = nullptr;
        // Pitch
        std::atomic<float>* pitchShiftSt      = nullptr;
        std::atomic<float>* detuneCents       = nullptr;
        std::atomic<float>* pitchRandomSt     = nullptr;
        std::atomic<float>* playbackRate      = nullptr;
        std::atomic<float>* transposeMode     = nullptr;
        std::atomic<float>* pitchQuantize     = nullptr;
        std::atomic<float>* formantShiftSt    = nullptr;
        std::atomic<float>* glideMs           = nullptr;
        // Amplitude
        std::atomic<float>* amplitude         = nullptr;
        std::atomic<float>* ampRandomDb       = nullptr;
        std::atomic<float>* adsrAttackMs      = nullptr;
        std::atomic<float>* adsrDecayMs       = nullptr;
        std::atomic<float>* adsrSustain       = nullptr;
        std::atomic<float>* adsrReleaseMs     = nullptr;
        std::atomic<float>* velocitySens      = nullptr;
        std::atomic<float>* crossfadeMs       = nullptr;
        // Spatial
        std::atomic<float>* pan               = nullptr;
        std::atomic<float>* panRandom         = nullptr;
        std::atomic<float>* stereoWidth       = nullptr;
        std::atomic<float>* voices            = nullptr;
        std::atomic<float>* voiceDetuneCents  = nullptr;
        // LFOs (4)
        std::array<std::atomic<float>*, 4> lfoRate   {};
        std::array<std::atomic<float>*, 4> lfoShape  {};
        std::array<std::atomic<float>*, 4> lfoDepth  {};
        std::array<std::atomic<float>*, 4> lfoPhase  {};
        // Env mods (2)
        std::array<std::atomic<float>*, 2> envAttack  {};
        std::array<std::atomic<float>*, 2> envDecay   {};
        std::array<std::atomic<float>*, 2> envSustain {};
        std::array<std::atomic<float>*, 2> envRelease {};
        // Env follower
        std::atomic<float>* efAttackMs        = nullptr;
        std::atomic<float>* efReleaseMs       = nullptr;
        // Filter
        std::atomic<float>* filterType        = nullptr;
        std::atomic<float>* filterCutoffHz    = nullptr;
        std::atomic<float>* filterResonance   = nullptr;
        std::atomic<float>* filterEnvDepth    = nullptr;
        std::atomic<float>* filterLfoDepth    = nullptr;
        std::atomic<float>* filterKeytrack    = nullptr;
        // Buffer
        std::atomic<float>* bufferLengthMs    = nullptr;
        std::atomic<float>* recordMode        = nullptr;
        std::atomic<float>* recordFeedback    = nullptr;
        std::atomic<float>* inputGainDb       = nullptr;
        // Effects
        std::atomic<float>* fxDrive           = nullptr;
        std::atomic<float>* fxWsType          = nullptr;
        std::array<std::atomic<float>*, 4> eqFreq  {};
        std::array<std::atomic<float>*, 4> eqGain  {};
        std::array<std::atomic<float>*, 4> eqQ     {};
        std::atomic<float>* chorusRate         = nullptr;
        std::atomic<float>* chorusDepth        = nullptr;
        std::atomic<float>* chorusMix          = nullptr;
        std::atomic<float>* delayTimeMs        = nullptr;
        std::atomic<float>* delayFeedback      = nullptr;
        std::atomic<float>* delayMix           = nullptr;
        std::atomic<float>* delayPingPong      = nullptr;
        std::atomic<float>* reverbRoom         = nullptr;
        std::atomic<float>* reverbDamp         = nullptr;
        std::atomic<float>* reverbMix          = nullptr;
        std::atomic<float>* limiterThresh      = nullptr;
        std::atomic<float>* limiterRelease     = nullptr;
        // Output
        std::atomic<float>* masterVolumeDb     = nullptr;
        std::atomic<float>* dryWet             = nullptr;
        std::atomic<float>* dcFilter           = nullptr;
        std::atomic<float>* antiAliasing       = nullptr;
    } raw;

private:
    juce::AudioProcessorValueTreeState apvts;
    GrainBuffer   grainBuffer;
    GranularSynth synth;
    ModMatrix     modMatrix;

    std::array<LFO, 4>          lfos;
    std::array<juce::ADSR, 2>   envMods;
    EnvelopeFollower            envFollower;
    EffectsChain                fxChain;

    juce::AudioFormatManager formatManager;
    juce::AudioBuffer<float> dryBuffer;
    double currentSampleRate = 44100.0;

    // Recording
    juce::TimeSliceThread recordingThread { "GranRecordThread" };
    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> recordWriter;
    std::atomic<bool>         isRecordingActive { false };
    std::atomic<juce::int64>  recordedSamples   { 0 };

    // DC filter state (single-pole HP per channel)
    std::array<float, 2> dcFilterPrev {};
    std::array<float, 2> dcFilterPrevOut {};

    // MIDI state for mod matrix sources
    float lastVelocity   = 0.0f;
    float lastAftertouch = 0.0f;
    float lastCC1        = 0.0f;

    void cacheRawParams();
    GranularParams buildParamSnapshot();
    void updateEffectsParams();
    void applyDCFilter(juce::AudioBuffer<float>& buf) noexcept;

public:
    void startRecording(const juce::File& destFile);
    void stopRecording();
    bool getIsRecording() const noexcept { return isRecordingActive.load(); }
    double getRecordedLengthSeconds() const noexcept
    {
        return currentSampleRate > 0.0
            ? static_cast<double>(recordedSamples.load()) / currentSampleRate : 0.0;
    }

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};
