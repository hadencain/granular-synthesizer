#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
// Parameter layout — all ~90 parameters
//==============================================================================

static juce::NormalisableRange<float> logRange(float min, float max, float centre)
{
    auto r = juce::NormalisableRange<float>(min, max);
    r.setSkewForCentre(centre);
    return r;
}

static juce::NormalisableRange<float> expRange(float min, float max, float skew)
{
    return juce::NormalisableRange<float>(min, max, 0.0f, skew);
}

juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    using P   = juce::AudioParameterFloat;
    using PI  = juce::AudioParameterInt;
    using PB  = juce::AudioParameterBool;
    using PC  = juce::AudioParameterChoice;
    using VT  = juce::AudioProcessorValueTreeState::ParameterLayout;
    using A   = juce::AudioParameterFloatAttributes;

    // Format helpers — these control what the slider text box shows
    auto fMs  = A().withStringFromValueFunction([](float v, int){ return juce::String(v, 1) + " ms"; });
    auto fHz  = A().withStringFromValueFunction([](float v, int){ return (v >= 100.0f ? juce::String(juce::roundToInt(v)) : juce::String(v, 1)) + " Hz"; });
    auto fHz2 = A().withStringFromValueFunction([](float v, int){ return juce::String(v, 2) + " Hz"; });
    auto fDb  = A().withStringFromValueFunction([](float v, int){ return juce::String(v, 1) + " dB"; });
    auto fPct = A().withStringFromValueFunction([](float v, int){ return juce::String(juce::roundToInt(v * 100)) + "%"; });
    auto fSt  = A().withStringFromValueFunction([](float v, int){ return juce::String(v, 1) + " st"; });
    auto fCt  = A().withStringFromValueFunction([](float v, int){ return juce::String(juce::roundToInt(v)) + " ct"; });
    auto fX   = A().withStringFromValueFunction([](float v, int){ return juce::String(v, 2) + "x"; });
    auto fQ   = A().withStringFromValueFunction([](float v, int){ return juce::String(v, 2); });
    auto fDeg = A().withStringFromValueFunction([](float v, int){ return juce::String(juce::roundToInt(v)) + "\xc2\xb0"; });
    auto fPan = A().withStringFromValueFunction([](float v, int) -> juce::String {
        if (std::abs(v) < 0.005f) return "C";
        int p = juce::roundToInt(std::abs(v) * 100);
        return juce::String(p) + (v < 0.0f ? "L" : "R");
    });
    auto fAmp = A().withStringFromValueFunction([](float v, int) -> juce::String {
        if (v <= 0.0001f) return "-inf dB";
        return juce::String(20.0f * std::log10(v), 1) + " dB";
    });

    VT layout;

    // --- Grain Core ---
    layout.add(std::make_unique<P>("grain_size_ms",      "Grain Size",        logRange(1.0f, 500.0f, 50.0f),   250.0f, fMs));
    layout.add(std::make_unique<P>("grain_density",      "Grain Density",     logRange(1.0f, 500.0f, 50.0f),    6.0f,  fHz));
    layout.add(std::make_unique<PC>("grain_envelope",    "Grain Envelope",    juce::StringArray{"Hann","Gaussian","Trapezoid","Rectangle","Triangular","Tukey"}, 0));
    layout.add(std::make_unique<P>("grain_overlap",      "Grain Overlap",     juce::NormalisableRange<float>(0.0f, 1.0f), 0.25f, fPct));
    layout.add(std::make_unique<P>("interonset_ms",      "Interonset Time",   logRange(1.0f, 1000.0f, 50.0f),  150.0f, fMs));
    layout.add(std::make_unique<PC>("grain_direction",   "Grain Direction",   juce::StringArray{"Forward","Reverse","Bidirectional","Random"}, 0));
    layout.add(std::make_unique<P>("randomize_size_ms",  "Randomize Size",    juce::NormalisableRange<float>(0.0f, 250.0f), 0.0f, fMs));
    layout.add(std::make_unique<P>("randomize_density",  "Randomize Density", juce::NormalisableRange<float>(0.0f, 200.0f), 0.0f, fHz));
    layout.add(std::make_unique<P>("grain_probability",  "Grain Probability", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f, fPct));

    // --- Playhead ---
    layout.add(std::make_unique<P>("position",           "Position",          juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f, fPct));
    layout.add(std::make_unique<P>("spray_ms",           "Spray",             juce::NormalisableRange<float>(0.0f, 500.0f), 0.0f, fMs));
    layout.add(std::make_unique<P>("scan_rate_hz",       "Scan Rate",         logRange(0.001f, 10.0f, 0.3f),   0.001f, fHz2));
    layout.add(std::make_unique<PC>("scan_shape",        "Scan Shape",        juce::StringArray{"Forward","Reverse","Pendulum","Random Walk","Sine"}, 0));
    layout.add(std::make_unique<P>("loop_start",         "Loop Start",        juce::NormalisableRange<float>(0.0f, 0.99f), 0.0f, fPct));
    layout.add(std::make_unique<P>("loop_end",           "Loop End",          juce::NormalisableRange<float>(0.01f, 1.0f), 1.0f, fPct));
    layout.add(std::make_unique<PB>("freeze",            "Freeze",            false));
    layout.add(std::make_unique<P>("scrub",              "Scrub",             juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f, fPct));

    // --- Pitch ---
    layout.add(std::make_unique<P>("pitch_shift_st",     "Pitch Shift",       juce::NormalisableRange<float>(-48.0f, 48.0f), 0.0f, fSt));
    layout.add(std::make_unique<P>("detune_cents",       "Detune",            juce::NormalisableRange<float>(-100.0f, 100.0f), 0.0f, fCt));
    layout.add(std::make_unique<P>("pitch_random_st",    "Pitch Random",      juce::NormalisableRange<float>(0.0f, 24.0f), 0.0f, fSt));
    layout.add(std::make_unique<P>("playback_rate",      "Playback Rate",     logRange(0.01f, 4.0f, 1.0f),     1.0f,   fX));
    layout.add(std::make_unique<PC>("transpose_mode",    "Transpose Mode",    juce::StringArray{"Doppler","Interpolated","Phase Vocoder"}, 0));
    layout.add(std::make_unique<PC>("pitch_quantize",    "Pitch Quantize",    juce::StringArray{"Off","Chromatic","Major","Minor","Custom"}, 0));
    layout.add(std::make_unique<P>("formant_shift_st",   "Formant Shift",     juce::NormalisableRange<float>(-12.0f, 12.0f), 0.0f, fSt));
    layout.add(std::make_unique<P>("glide_ms",           "Glide",             logRange(0.0f, 2000.0f, 100.0f), 0.0f,   fMs));

    // --- Amplitude ---
    layout.add(std::make_unique<P>("amplitude",          "Amplitude",         expRange(0.0f, 1.0f, 0.25f),     1.0f,   fAmp));
    layout.add(std::make_unique<P>("amp_random_db",      "Amp Random",        juce::NormalisableRange<float>(0.0f, 18.0f), 0.0f, fDb));
    layout.add(std::make_unique<P>("adsr_attack_ms",     "Attack",            logRange(0.0f, 5000.0f, 100.0f),  5.0f,  fMs));
    layout.add(std::make_unique<P>("adsr_decay_ms",      "Decay",             logRange(0.0f, 5000.0f, 200.0f), 100.0f, fMs));
    layout.add(std::make_unique<P>("adsr_sustain",       "Sustain",           juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f, fPct));
    layout.add(std::make_unique<P>("adsr_release_ms",    "Release",           logRange(0.0f, 10000.0f, 500.0f), 150.0f, fMs));
    layout.add(std::make_unique<P>("velocity_sensitivity","Velocity Sens",    juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f, fPct));
    layout.add(std::make_unique<P>("crossfade_ms",       "Crossfade",         juce::NormalisableRange<float>(0.0f, 50.0f), 5.0f, fMs));

    // --- Spatialization ---
    layout.add(std::make_unique<P>("pan",                "Pan",               juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f, fPan));
    layout.add(std::make_unique<P>("pan_random",         "Pan Random",        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f, fPct));
    layout.add(std::make_unique<P>("stereo_width",       "Stereo Width",      juce::NormalisableRange<float>(0.0f, 2.0f), 1.0f, fPct));
    layout.add(std::make_unique<PI>("voices",            "Voices",            1, 16, 1));
    layout.add(std::make_unique<P>("voice_detune_cents", "Voice Detune",      juce::NormalisableRange<float>(0.0f, 100.0f), 0.0f, fCt));

    // --- LFOs (4) ---
    for (int n = 1; n <= 4; ++n)
    {
        auto ns = juce::String(n);
        layout.add(std::make_unique<P>("lfo" + ns + "_rate_hz",  "LFO " + ns + " Rate",  logRange(0.01f, 100.0f, 5.0f), 1.0f, fHz));
        layout.add(std::make_unique<PC>("lfo" + ns + "_shape",   "LFO " + ns + " Shape", juce::StringArray{"Sine","Triangle","Saw Up","Saw Down","Square","S&H","Smooth Rnd"}, 0));
        layout.add(std::make_unique<P>("lfo" + ns + "_depth",    "LFO " + ns + " Depth", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f, fPct));
        layout.add(std::make_unique<P>("lfo" + ns + "_phase_deg","LFO " + ns + " Phase", juce::NormalisableRange<float>(0.0f, 360.0f), 0.0f, fDeg));
    }

    // --- Envelope Modulators (2) ---
    for (int n = 1; n <= 2; ++n)
    {
        auto ns = juce::String(n);
        layout.add(std::make_unique<P>("envmod" + ns + "_attack_ms",  "EnvMod " + ns + " Attack",  logRange(0.0f, 5000.0f, 100.0f), 10.0f,  fMs));
        layout.add(std::make_unique<P>("envmod" + ns + "_decay_ms",   "EnvMod " + ns + " Decay",   logRange(0.0f, 5000.0f, 200.0f), 100.0f, fMs));
        layout.add(std::make_unique<P>("envmod" + ns + "_sustain",    "EnvMod " + ns + " Sustain", juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f, fPct));
        layout.add(std::make_unique<P>("envmod" + ns + "_release_ms", "EnvMod " + ns + " Release", logRange(0.0f, 10000.0f, 500.0f), 500.0f, fMs));
    }

    // --- Envelope Follower ---
    layout.add(std::make_unique<P>("ef_attack_ms",       "EF Attack",         logRange(1.0f, 500.0f, 50.0f),  10.0f,  fMs));
    layout.add(std::make_unique<P>("ef_release_ms",      "EF Release",        logRange(1.0f, 2000.0f, 200.0f),100.0f, fMs));

    // --- Filter ---
    layout.add(std::make_unique<PC>("filter_type",       "Filter Type",       juce::StringArray{"Low-Pass","High-Pass","Band-Pass","Notch"}, 0));
    layout.add(std::make_unique<P>("filter_cutoff_hz",   "Filter Cutoff",     logRange(20.0f, 20000.0f, 1000.0f), 20000.0f, fHz));
    layout.add(std::make_unique<P>("filter_resonance",   "Filter Resonance",  logRange(0.1f, 40.0f, 2.0f),    0.707f, fQ));
    layout.add(std::make_unique<P>("filter_env_depth",   "Filter Env Depth",  juce::NormalisableRange<float>(-20000.0f, 20000.0f), 0.0f, fHz));
    layout.add(std::make_unique<P>("filter_lfo_depth",   "Filter LFO Depth",  juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f, fPct));
    layout.add(std::make_unique<P>("filter_keytrack",    "Filter Keytrack",   juce::NormalisableRange<float>(0.0f, 2.0f), 0.0f, fPct));

    // --- Buffer ---
    layout.add(std::make_unique<P>("buffer_length_ms",   "Buffer Length",     logRange(10.0f, 60000.0f, 2000.0f), 2000.0f, fMs));
    layout.add(std::make_unique<PC>("record_mode",       "Record Mode",       juce::StringArray{"Off","Continuous","One-Shot","Gate"}, 0));
    layout.add(std::make_unique<P>("record_feedback",    "Record Feedback",   juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f, fPct));
    layout.add(std::make_unique<P>("input_gain_db",      "Input Gain",        juce::NormalisableRange<float>(-24.0f, 24.0f), 0.0f, fDb));

    // --- Effects ---
    layout.add(std::make_unique<P>("fx_drive",           "Drive",             juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f, fPct));
    layout.add(std::make_unique<PC>("fx_ws_type",        "Waveshaper Type",   juce::StringArray{"Tanh","Soft Clip"}, 0));
    for (int n = 1; n <= 4; ++n)
    {
        auto ns = juce::String(n);
        const float defaultFreqs[] = { 100.0f, 500.0f, 2000.0f, 8000.0f };
        layout.add(std::make_unique<P>("fx_eq" + ns + "_freq", "EQ " + ns + " Freq", logRange(20.0f, 20000.0f, 1000.0f), defaultFreqs[n-1], fHz));
        layout.add(std::make_unique<P>("fx_eq" + ns + "_gain", "EQ " + ns + " Gain", juce::NormalisableRange<float>(-18.0f, 18.0f), 0.0f, fDb));
        layout.add(std::make_unique<P>("fx_eq" + ns + "_q",    "EQ " + ns + " Q",    logRange(0.1f, 10.0f, 0.707f), 0.707f, fQ));
    }
    layout.add(std::make_unique<P>("fx_chorus_rate",     "Chorus Rate",       logRange(0.01f, 10.0f, 0.5f),   0.5f,  fHz));
    layout.add(std::make_unique<P>("fx_chorus_depth",    "Chorus Depth",      juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f, fPct));
    layout.add(std::make_unique<P>("fx_chorus_mix",      "Chorus Mix",        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f, fPct));
    layout.add(std::make_unique<P>("fx_delay_time_ms",   "Delay Time",        logRange(1.0f, 2000.0f, 250.0f), 250.0f, fMs));
    layout.add(std::make_unique<P>("fx_delay_feedback",  "Delay Feedback",    juce::NormalisableRange<float>(0.0f, 0.99f), 0.0f, fPct));
    layout.add(std::make_unique<P>("fx_delay_mix",       "Delay Mix",         juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f, fPct));
    layout.add(std::make_unique<PB>("fx_delay_pingpong", "Delay Ping-Pong",   false));
    layout.add(std::make_unique<P>("fx_reverb_room",     "Reverb Room",       juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f, fPct));
    layout.add(std::make_unique<P>("fx_reverb_damp",     "Reverb Damp",       juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f, fPct));
    layout.add(std::make_unique<P>("fx_reverb_mix",      "Reverb Mix",        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f, fPct));
    layout.add(std::make_unique<P>("fx_limiter_thresh",  "Limiter Threshold", juce::NormalisableRange<float>(-60.0f, 0.0f), -0.1f, fDb));
    layout.add(std::make_unique<P>("fx_limiter_release", "Limiter Release",   logRange(1.0f, 2000.0f, 100.0f), 100.0f, fMs));

    // --- Output ---
    { auto r = juce::NormalisableRange<float>(-60.0f, 6.0f); r.setSkewForCentre(-12.0f);
      layout.add(std::make_unique<P>("master_volume_db", "Master Volume", r, 0.0f, fDb)); }
    layout.add(std::make_unique<P>("dry_wet",            "Dry/Wet",           juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f, fPct));
    layout.add(std::make_unique<PB>("dc_filter",         "DC Filter",         true));
    layout.add(std::make_unique<PC>("anti_aliasing",     "Anti-Aliasing",     juce::StringArray{"Off","Low","High"}, 1));

    return layout;
}

//==============================================================================
// PluginProcessor
//==============================================================================

PluginProcessor::PluginProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "GranulatorState", createParameterLayout())
{
    formatManager.registerBasicFormats();
}

PluginProcessor::~PluginProcessor() = default;

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void PluginProcessor::cacheRawParams()
{
    auto* r = &raw;
    r->grainSizeMs      = apvts.getRawParameterValue("grain_size_ms");
    r->grainDensity     = apvts.getRawParameterValue("grain_density");
    r->grainEnvelope    = apvts.getRawParameterValue("grain_envelope");
    r->grainOverlap     = apvts.getRawParameterValue("grain_overlap");
    r->interonsetMs     = apvts.getRawParameterValue("interonset_ms");
    r->grainDirection   = apvts.getRawParameterValue("grain_direction");
    r->randomizeSizeMs  = apvts.getRawParameterValue("randomize_size_ms");
    r->randomizeDensity = apvts.getRawParameterValue("randomize_density");
    r->grainProbability = apvts.getRawParameterValue("grain_probability");

    r->position         = apvts.getRawParameterValue("position");
    r->sprayMs          = apvts.getRawParameterValue("spray_ms");
    r->scanRateHz       = apvts.getRawParameterValue("scan_rate_hz");
    r->scanShape        = apvts.getRawParameterValue("scan_shape");
    r->loopStart        = apvts.getRawParameterValue("loop_start");
    r->loopEnd          = apvts.getRawParameterValue("loop_end");
    r->freeze           = apvts.getRawParameterValue("freeze");
    r->scrub            = apvts.getRawParameterValue("scrub");

    r->pitchShiftSt     = apvts.getRawParameterValue("pitch_shift_st");
    r->detuneCents      = apvts.getRawParameterValue("detune_cents");
    r->pitchRandomSt    = apvts.getRawParameterValue("pitch_random_st");
    r->playbackRate     = apvts.getRawParameterValue("playback_rate");
    r->transposeMode    = apvts.getRawParameterValue("transpose_mode");
    r->pitchQuantize    = apvts.getRawParameterValue("pitch_quantize");
    r->formantShiftSt   = apvts.getRawParameterValue("formant_shift_st");
    r->glideMs          = apvts.getRawParameterValue("glide_ms");

    r->amplitude        = apvts.getRawParameterValue("amplitude");
    r->ampRandomDb      = apvts.getRawParameterValue("amp_random_db");
    r->adsrAttackMs     = apvts.getRawParameterValue("adsr_attack_ms");
    r->adsrDecayMs      = apvts.getRawParameterValue("adsr_decay_ms");
    r->adsrSustain      = apvts.getRawParameterValue("adsr_sustain");
    r->adsrReleaseMs    = apvts.getRawParameterValue("adsr_release_ms");
    r->velocitySens     = apvts.getRawParameterValue("velocity_sensitivity");
    r->crossfadeMs      = apvts.getRawParameterValue("crossfade_ms");

    r->pan              = apvts.getRawParameterValue("pan");
    r->panRandom        = apvts.getRawParameterValue("pan_random");
    r->stereoWidth      = apvts.getRawParameterValue("stereo_width");
    r->voices           = apvts.getRawParameterValue("voices");
    r->voiceDetuneCents = apvts.getRawParameterValue("voice_detune_cents");

    for (int n = 0; n < 4; ++n)
    {
        auto ns = juce::String(n + 1);
        r->lfoRate[n]  = apvts.getRawParameterValue("lfo" + ns + "_rate_hz");
        r->lfoShape[n] = apvts.getRawParameterValue("lfo" + ns + "_shape");
        r->lfoDepth[n] = apvts.getRawParameterValue("lfo" + ns + "_depth");
        r->lfoPhase[n] = apvts.getRawParameterValue("lfo" + ns + "_phase_deg");
    }
    for (int n = 0; n < 2; ++n)
    {
        auto ns = juce::String(n + 1);
        r->envAttack[n]  = apvts.getRawParameterValue("envmod" + ns + "_attack_ms");
        r->envDecay[n]   = apvts.getRawParameterValue("envmod" + ns + "_decay_ms");
        r->envSustain[n] = apvts.getRawParameterValue("envmod" + ns + "_sustain");
        r->envRelease[n] = apvts.getRawParameterValue("envmod" + ns + "_release_ms");
    }

    r->efAttackMs       = apvts.getRawParameterValue("ef_attack_ms");
    r->efReleaseMs      = apvts.getRawParameterValue("ef_release_ms");

    r->filterType       = apvts.getRawParameterValue("filter_type");
    r->filterCutoffHz   = apvts.getRawParameterValue("filter_cutoff_hz");
    r->filterResonance  = apvts.getRawParameterValue("filter_resonance");
    r->filterEnvDepth   = apvts.getRawParameterValue("filter_env_depth");
    r->filterLfoDepth   = apvts.getRawParameterValue("filter_lfo_depth");
    r->filterKeytrack   = apvts.getRawParameterValue("filter_keytrack");

    r->bufferLengthMs   = apvts.getRawParameterValue("buffer_length_ms");
    r->recordMode       = apvts.getRawParameterValue("record_mode");
    r->recordFeedback   = apvts.getRawParameterValue("record_feedback");
    r->inputGainDb      = apvts.getRawParameterValue("input_gain_db");

    r->fxDrive          = apvts.getRawParameterValue("fx_drive");
    r->fxWsType         = apvts.getRawParameterValue("fx_ws_type");
    for (int n = 0; n < 4; ++n)
    {
        auto ns = juce::String(n + 1);
        r->eqFreq[n] = apvts.getRawParameterValue("fx_eq" + ns + "_freq");
        r->eqGain[n] = apvts.getRawParameterValue("fx_eq" + ns + "_gain");
        r->eqQ[n]    = apvts.getRawParameterValue("fx_eq" + ns + "_q");
    }
    r->chorusRate       = apvts.getRawParameterValue("fx_chorus_rate");
    r->chorusDepth      = apvts.getRawParameterValue("fx_chorus_depth");
    r->chorusMix        = apvts.getRawParameterValue("fx_chorus_mix");
    r->delayTimeMs      = apvts.getRawParameterValue("fx_delay_time_ms");
    r->delayFeedback    = apvts.getRawParameterValue("fx_delay_feedback");
    r->delayMix         = apvts.getRawParameterValue("fx_delay_mix");
    r->delayPingPong    = apvts.getRawParameterValue("fx_delay_pingpong");
    r->reverbRoom       = apvts.getRawParameterValue("fx_reverb_room");
    r->reverbDamp       = apvts.getRawParameterValue("fx_reverb_damp");
    r->reverbMix        = apvts.getRawParameterValue("fx_reverb_mix");
    r->limiterThresh    = apvts.getRawParameterValue("fx_limiter_thresh");
    r->limiterRelease   = apvts.getRawParameterValue("fx_limiter_release");

    r->masterVolumeDb   = apvts.getRawParameterValue("master_volume_db");
    r->dryWet           = apvts.getRawParameterValue("dry_wet");
    r->dcFilter         = apvts.getRawParameterValue("dc_filter");
    r->antiAliasing     = apvts.getRawParameterValue("anti_aliasing");
}

void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    cacheRawParams();

    currentSampleRate = sampleRate;
    grainBuffer.prepare(sampleRate);

    const juce::dsp::ProcessSpec spec { sampleRate,
        static_cast<juce::uint32>(samplesPerBlock), 2 };

    synth.prepare(spec, &grainBuffer);

    for (auto& lfo : lfos)   lfo.prepare(sampleRate);
    for (auto& env : envMods) env.setSampleRate(sampleRate);
    envFollower.prepare(sampleRate);

    fxChain.prepare(spec);

    dryBuffer.setSize(2, samplesPerBlock);
    dcFilterPrev.fill(0.0f);
    dcFilterPrevOut.fill(0.0f);
}

void PluginProcessor::releaseResources()
{
    fxChain.reset();
}

GranularParams PluginProcessor::buildParamSnapshot()
{
    GranularParams p;
    const auto& r = raw;

    p.grainSizeMs       = r.grainSizeMs->load();
    p.grainDensity      = r.grainDensity->load();
    p.grainEnvelopeShape = static_cast<int>(r.grainEnvelope->load());
    p.grainOverlap      = r.grainOverlap->load();
    p.interonsetMs      = r.interonsetMs->load();
    p.grainDirection    = static_cast<int>(r.grainDirection->load());
    p.randomizeSizeMs   = r.randomizeSizeMs->load();
    p.randomizeDensity  = r.randomizeDensity->load();
    p.grainProbability  = r.grainProbability->load();

    p.position          = r.position->load();
    p.sprayMs           = r.sprayMs->load();
    p.scanRateHz        = r.scanRateHz->load();
    p.scanShape         = static_cast<int>(r.scanShape->load());
    p.loopStart         = r.loopStart->load();
    p.loopEnd           = r.loopEnd->load();
    p.freeze            = r.freeze->load() > 0.5f;
    p.scrub             = r.scrub->load();

    p.pitchShiftSemitones   = r.pitchShiftSt->load();
    p.detuneCents           = r.detuneCents->load();
    p.pitchRandomSemitones  = r.pitchRandomSt->load();
    p.playbackRate          = r.playbackRate->load();
    p.transposeMode         = static_cast<int>(r.transposeMode->load());
    p.pitchQuantize         = static_cast<int>(r.pitchQuantize->load());
    p.formantShiftSemitones = r.formantShiftSt->load();
    p.glideMs               = r.glideMs->load();

    p.amplitude             = r.amplitude->load();
    p.ampRandomDb           = r.ampRandomDb->load();
    p.adsrAttackMs          = r.adsrAttackMs->load();
    p.adsrDecayMs           = r.adsrDecayMs->load();
    p.adsrSustain           = r.adsrSustain->load();
    p.adsrReleaseMs         = r.adsrReleaseMs->load();
    p.velocitySensitivity   = r.velocitySens->load();
    p.crossfadeMs           = r.crossfadeMs->load();

    p.pan                   = r.pan->load();
    p.panRandom             = r.panRandom->load();
    p.stereoWidth           = r.stereoWidth->load();
    p.voices                = static_cast<int>(r.voices->load());
    p.voiceDetuneCents      = r.voiceDetuneCents->load();

    p.filterType            = static_cast<int>(r.filterType->load());
    p.filterCutoffHz        = r.filterCutoffHz->load();
    p.filterResonance       = r.filterResonance->load();
    p.filterEnvDepth        = r.filterEnvDepth->load();
    p.filterLfoDepth        = r.filterLfoDepth->load();
    p.filterKeytrack        = r.filterKeytrack->load();

    p.bufferLengthMs        = r.bufferLengthMs->load();
    p.recordMode            = static_cast<int>(r.recordMode->load());
    p.recordFeedback        = r.recordFeedback->load();
    p.inputGainDb           = r.inputGainDb->load();

    p.masterVolumeDb        = r.masterVolumeDb->load();
    p.dryWet                = r.dryWet->load();
    p.dcFilter              = r.dcFilter->load() > 0.5f;
    p.antiAliasing          = static_cast<int>(r.antiAliasing->load());

    // Apply mod matrix
    p.modGrainSize       = modMatrix.evaluate(ModTarget::GrainSize);
    p.modGrainDensity    = modMatrix.evaluate(ModTarget::GrainDensity);
    p.modPitch           = modMatrix.evaluate(ModTarget::GrainPitch);
    p.modAmplitude       = modMatrix.evaluate(ModTarget::GrainAmplitude);
    p.modPan             = modMatrix.evaluate(ModTarget::GrainPan);
    p.modPosition        = modMatrix.evaluate(ModTarget::Position);
    p.modSpray           = modMatrix.evaluate(ModTarget::Spray);
    p.modScanRate        = modMatrix.evaluate(ModTarget::ScanRate);
    p.modFilterCutoff    = modMatrix.evaluate(ModTarget::FilterCutoff);
    p.modFilterResonance = modMatrix.evaluate(ModTarget::FilterResonance);

    // Write all mod values for UI display
    for (int i = 0; i < static_cast<int>(ModTarget::COUNT); ++i)
        modDisplayValues[i].store(modMatrix.evaluate(static_cast<ModTarget>(i)));

    return p;
}

void PluginProcessor::updateEffectsParams()
{
    const auto& r = raw;

    // Waveshaper
    const int wsType = static_cast<int>(r.fxWsType->load());
    fxChain.setWaveshaperType(wsType);

    // Chorus
    fxChain.chorus.setRate(r.chorusRate->load());
    fxChain.chorus.setDepth(r.chorusDepth->load());
    fxChain.chorus.setMix(r.chorusMix->load());

    // Delay params cached in EffectsChain
    fxChain.delayMix         = r.delayMix->load();
    fxChain.delayFb          = r.delayFeedback->load();
    fxChain.pingPong         = r.delayPingPong->load() > 0.5f;
    fxChain.delayTimeSamples = juce::jlimit(1.0f,
        static_cast<float>(fxChain.delayLine.getMaximumDelayInSamples() - 1),
        r.delayTimeMs->load() / 1000.0f * static_cast<float>(getSampleRate()));

    // Reverb
    juce::dsp::Reverb::Parameters rp;
    rp.roomSize  = r.reverbRoom->load();
    rp.damping   = r.reverbDamp->load();
    const float rvbMix = r.reverbMix->load();
    rp.wetLevel  = rvbMix;
    rp.dryLevel  = 1.0f - rvbMix;
    rp.width     = 1.0f;
    fxChain.reverb.setParameters(rp);
    fxChain.reverbMix = rvbMix;

    // Limiter
    fxChain.limiter.setThreshold(r.limiterThresh->load());
    fxChain.limiter.setRelease(r.limiterRelease->load());

    // Drive
    fxChain.drive = r.fxDrive->load();
}

void PluginProcessor::applyDCFilter(juce::AudioBuffer<float>& buf) noexcept
{
    // Single-pole high-pass at ~5 Hz: y[n] = x[n] - x[n-1] + 0.9998 * y[n-1]
    constexpr float coeff = 0.9998f;
    for (int ch = 0; ch < buf.getNumChannels(); ++ch)
    {
        float* data = buf.getWritePointer(ch);
        float prev    = dcFilterPrev[ch];
        float prevOut = dcFilterPrevOut[ch];
        for (int i = 0; i < buf.getNumSamples(); ++i)
        {
            const float in  = data[i];
            const float out = in - prev + coeff * prevOut;
            data[i] = out;
            prev    = in;
            prevOut = out;
        }
        dcFilterPrev[ch]    = prev;
        dcFilterPrevOut[ch] = prevOut;
    }
}

void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                    juce::MidiBuffer& midiBuffer)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();

    // 1. Save dry signal
    dryBuffer.setSize(2, numSamples, false, false, true);
    for (int ch = 0; ch < 2; ++ch)
        dryBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);

    // 2. Input gain
    const float inputGainLin = juce::Decibels::decibelsToGain(raw.inputGainDb->load());
    buffer.applyGain(inputGainLin);

    // 3. Write to ring buffer
    const auto recMode = static_cast<GrainBuffer::RecordMode>(
        static_cast<int>(raw.recordMode->load()));
    grainBuffer.setRecordMode(recMode);
    grainBuffer.writeSamples(buffer, 0, numSamples, raw.recordFeedback->load());

    // 4. Process MIDI for mod sources and synth
    for (const auto msg : midiBuffer)
    {
        const auto m = msg.getMessage();
        if (m.isNoteOn())
        {
            lastVelocity = m.getFloatVelocity();
            for (auto& env : envMods) env.noteOn();
        }
        if (m.isNoteOff())
        {
            for (auto& env : envMods) env.noteOff();
        }
        if (m.isAftertouch())  lastAftertouch = m.getAfterTouchValue() / 127.0f;
        if (m.isController() && m.getControllerNumber() == 1)
            lastCC1 = m.getControllerValue() / 127.0f;
    }

    // 5. Tick LFOs and envelope followers
    for (int i = 0; i < 4; ++i)
    {
        lfos[i].setRate(raw.lfoRate[i]->load());
        lfos[i].setShape(static_cast<LFOShape>(static_cast<int>(raw.lfoShape[i]->load())));
        lfos[i].setPhaseOffset(raw.lfoPhase[i]->load());
        lfos[i].processBlock(numSamples);
    }
    envFollower.setAttackMs(raw.efAttackMs->load());
    envFollower.setReleaseMs(raw.efReleaseMs->load());
    // Run envelope follower on first channel of input
    {
        const float* inData = dryBuffer.getReadPointer(0);
        for (int i = 0; i < numSamples; ++i)
            envFollower.processSample(inData[i]);
    }

    // Tick envelope modulators and capture end-of-block output
    float envModOutput[2] = { 0.0f, 0.0f };
    for (int i = 0; i < 2; ++i)
    {
        juce::ADSR::Parameters p;
        p.attack  = juce::jmax(0.001f, raw.envAttack[i]->load()  / 1000.0f);
        p.decay   = juce::jmax(0.001f, raw.envDecay[i]->load()   / 1000.0f);
        p.sustain = raw.envSustain[i]->load();
        p.release = juce::jmax(0.001f, raw.envRelease[i]->load() / 1000.0f);
        envMods[i].setParameters(p);
        for (int s = 0; s < numSamples; ++s)
            envModOutput[i] = envMods[i].getNextSample();
    }

    // 6. Update mod matrix sources
    ModSources modSrc;
    for (int i = 0; i < 4; ++i)
        modSrc.lfo[i] = lfos[i].getCurrentValue() * raw.lfoDepth[i]->load();
    modSrc.velocity    = lastVelocity;
    modSrc.aftertouch  = lastAftertouch;
    modSrc.cc1         = lastCC1;
    modSrc.envFollower = envFollower.getCurrentValue();
    modSrc.env[0]      = envModOutput[0];
    modSrc.env[1]      = envModOutput[1];
    modSrc.random      = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
    modMatrix.syncPending();
    modMatrix.updateSources(modSrc);

    // 7. Build param snapshot and update synth
    const GranularParams params = buildParamSnapshot();
    synth.updateParams(params);

    // 8. Clear output and render synth
    buffer.clear();
    synth.renderNextBlock(buffer, midiBuffer, 0, numSamples);

    // 9. Effects chain
    updateEffectsParams();
    fxChain.process(buffer);

    // 10. Drive waveshaping (pre-fx)
    if (params.dryWet > 0.001f) {}  // placeholder for waveshaper drive

    // 11. Dry/wet blend
    const float wet = raw.dryWet->load();
    const float dry = 1.0f - wet;
    if (dry > 0.001f)
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            buffer.applyGain(ch, 0, numSamples, wet);
            buffer.addFrom(ch, 0, dryBuffer, ch, 0, numSamples, dry);
        }
    }

    // 12. Master volume
    const float masterGain = juce::Decibels::decibelsToGain(
        juce::jlimit(-100.0f, 6.0f, raw.masterVolumeDb->load()));
    buffer.applyGain(masterGain);

    // 13. DC filter
    if (raw.dcFilter->load() > 0.5f)
        applyDCFilter(buffer);

    // Write to recorder if active
    if (isRecordingActive.load() && recordWriter)
    {
        recordWriter->write(buffer.getArrayOfReadPointers(), buffer.getNumSamples());
        recordedSamples.fetch_add(buffer.getNumSamples());
    }
}

void PluginProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto apvtsXml = apvts.copyState().createXml();
    auto modXml   = modMatrix.toValueTree().createXml();

    juce::XmlElement root("GranulatorState");
    root.addChildElement(apvtsXml.release());
    root.addChildElement(modXml.release());
    copyXmlToBinary(root, destData);
}

void PluginProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        if (auto* apvtsXml = xml->getChildByName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*apvtsXml));

        if (auto* modXml = xml->getChildByName("ModMatrix"))
            modMatrix.fromValueTree(juce::ValueTree::fromXml(*modXml));
    }
}

void PluginProcessor::startRecording(const juce::File& destFile)
{
    stopRecording();

    if (!recordingThread.isThreadRunning())
        recordingThread.startThread();

    juce::WavAudioFormat wav;
    auto fos = std::make_unique<juce::FileOutputStream>(destFile);
    if (!fos->openedOk()) return;

    auto* writer = wav.createWriterFor(fos.release(), currentSampleRate, 2, 24, {}, 0);
    if (!writer) return;

    recordedSamples.store(0);
    recordWriter = std::make_unique<juce::AudioFormatWriter::ThreadedWriter>(
        writer, recordingThread, 65536);
    isRecordingActive.store(true);
}

void PluginProcessor::stopRecording()
{
    isRecordingActive.store(false);
    recordWriter.reset();
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
