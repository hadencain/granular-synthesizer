#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// Scan shapes for playhead movement
enum class ScanShape { Forward = 0, Reverse, Pendulum, RandomWalk, Sine };

// Grain direction modes
enum class GrainDirection { Forward = 0, Reverse, Bidirectional, Random };

// LFO waveform shapes
enum class LFOShape { Sine = 0, Triangle, SawUp, SawDown, Square, SampleHold, SmoothRandom };

// Mod matrix source IDs
enum class ModSource {
    LFO1 = 0, LFO2, LFO3, LFO4,
    Env1, Env2,
    Velocity, Aftertouch, CC1,
    EnvFollower, Random,
    COUNT
};

// Mod matrix target IDs
enum class ModTarget {
    GrainSize = 0, GrainDensity, GrainPitch, GrainAmplitude, GrainPan,
    Position, Spray, ScanRate,
    FilterCutoff, FilterResonance,
    PitchShift, Detune, PlaybackRate,
    Amplitude, MasterVolume,
    COUNT
};

// POD snapshot of all parameters for one audio block — passed to engine/voice.
// Built in PluginProcessor::processBlock() from APVTS raw pointers.
struct GranularParams
{
    // Grain core
    float grainSizeMs       = 80.0f;
    float grainDensity      = 40.0f;
    int   grainEnvelopeShape = 0;       // GrainEnvelope::Shape cast
    float grainOverlap      = 0.5f;
    float interonsetMs      = 25.0f;
    int   grainDirection    = 0;        // GrainDirection cast
    float randomizeSizeMs   = 0.0f;
    float randomizeDensity  = 0.0f;

    // Playhead
    float position          = 0.0f;    // 0..1
    float sprayMs           = 0.0f;
    float scanRateHz        = 0.1f;
    int   scanShape         = 0;       // ScanShape cast
    float loopStart         = 0.0f;
    float loopEnd           = 1.0f;
    bool  freeze            = false;
    float scrub             = 0.0f;

    // Pitch
    float pitchShiftSemitones   = 0.0f;
    float detuneCents           = 0.0f;
    float pitchRandomSemitones  = 0.0f;
    float playbackRate          = 1.0f;
    int   transposeMode         = 0;
    int   pitchQuantize         = 0;
    float formantShiftSemitones = 0.0f;
    float glideMs               = 0.0f;

    // Amplitude
    float amplitude         = 1.0f;
    float ampRandomDb       = 0.0f;
    float adsrAttackMs      = 10.0f;
    float adsrDecayMs       = 100.0f;
    float adsrSustain       = 0.8f;
    float adsrReleaseMs     = 500.0f;
    float velocitySensitivity = 1.0f;
    float crossfadeMs       = 5.0f;

    // Spatialization
    float pan               = 0.0f;
    float panRandom         = 0.0f;
    float stereoWidth       = 1.0f;
    int   voices            = 1;
    float voiceDetuneCents  = 0.0f;

    // Filter
    int   filterType        = 0;       // LP/HP/BP/Notch
    float filterCutoffHz    = 20000.0f;
    float filterResonance   = 0.707f;
    float filterEnvDepth    = 0.0f;
    float filterLfoDepth    = 0.0f;
    float filterKeytrack    = 0.0f;

    // Buffer
    float bufferLengthMs    = 2000.0f;
    int   recordMode        = 0;
    float recordFeedback    = 0.0f;
    float inputGainDb       = 0.0f;

    // Output
    float masterVolumeDb    = 0.0f;
    float dryWet            = 1.0f;
    bool  dcFilter          = true;
    int   antiAliasing      = 1;

    // Runtime mod offsets (filled in by ModMatrix evaluation, added to base params)
    float modGrainSize      = 0.0f;
    float modGrainDensity   = 0.0f;
    float modPitch          = 0.0f;
    float modAmplitude      = 0.0f;
    float modPan            = 0.0f;
    float modPosition       = 0.0f;
    float modSpray          = 0.0f;
    float modScanRate       = 0.0f;
    float modFilterCutoff   = 0.0f;
    float modFilterResonance = 0.0f;

    // Per-voice runtime values (set by GranularVoice)
    float midiNote          = 60.0f;
    float velocity          = 1.0f;
    float voiceDetuneOffset = 0.0f;   // cents, distributed by GranularSynth
};
