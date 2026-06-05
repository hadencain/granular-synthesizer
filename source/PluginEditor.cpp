#include "PluginEditor.h"

static constexpr int kW        = 1440;
static constexpr int kH        = 720;
static constexpr int kHeaderH  = 44;
static constexpr int kWaveH    = 72;
static constexpr int kBodyY    = kHeaderH + kWaveH;
static constexpr int kBodyH    = kH - kBodyY;       // 604
static constexpr int kLfoColW  = 280;
static constexpr int kLfoFootH = 90;
static constexpr int kMainW    = kW - kLfoColW;     // 1160
static constexpr int kColW     = kMainW / 4;        // 290

PluginEditor::PluginEditor(PluginProcessor& p)
    : AudioProcessorEditor(&p),
      processor(p),
      grainSection    (p.getAPVTS()),
      playheadSection (p.getAPVTS()),
      pitchSection    (p.getAPVTS()),
      ampSection      (p.getAPVTS()),
      spatialSection  (p.getAPVTS()),
      filterSection   (p.getAPVTS()),
      bufferSection   (p.getAPVTS()),
      fxSection       (p.getAPVTS()),
      modSection      (p.getAPVTS(), p.getModMatrix()),
      waveformDisplay (p.getFormatManager()),
      masterVolAtt    (p.getAPVTS(), "master_volume_db", masterVolSlider)
{
    setLookAndFeel(&laf);
    setSize(kW, kH);

    titleLabel.setText("granular synthesizer", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, GranularLookAndFeel::accent);
    titleLabel.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    titleLabel.addMouseListener(this, false);
    addAndMakeVisible(titleLabel);

    addAndMakeVisible(grainDotDisplay);

    startBtn.setButtonText("START");
    startBtn.onClick = [this]() { processor.getSynth().noteOn(1, 60, 0.8f); };
    addAndMakeVisible(startBtn);

    stopBtn.setButtonText("STOP");
    stopBtn.onClick = [this]() { processor.getSynth().noteOff(1, 60, 0.8f, true); };
    addAndMakeVisible(stopBtn);

    waveformDisplay.setBuffer(&p.getGrainBuffer());
    waveformDisplay.setAPVTS(&p.getAPVTS());
    addAndMakeVisible(waveformDisplay);

    addAndMakeVisible(grainSection);
    addAndMakeVisible(playheadSection);
    addAndMakeVisible(pitchSection);
    addAndMakeVisible(ampSection);
    addAndMakeVisible(spatialSection);
    addAndMakeVisible(filterSection);
    addAndMakeVisible(bufferSection);
    addAndMakeVisible(fxSection);
    addAndMakeVisible(modSection);

    masterVolLabel.setText("GAIN", juce::dontSendNotification);
    masterVolLabel.setJustificationType(juce::Justification::centred);
    masterVolLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    masterVolLabel.setColour(juce::Label::textColourId, GranularLookAndFeel::textSecondary);

    masterVolSlider.setTextValueSuffix(" dB");
    masterVolSlider.setNumDecimalPlacesToDisplay(1);

    addAndMakeVisible(masterVolSlider);
    addAndMakeVisible(masterVolLabel);

    waveformDisplay.onLoadFile = [&](const juce::File& file) {
        p.getGrainBuffer().loadFile(file, p.getFormatManager());
    };

    // Knob → ModTarget binding table. scale = native-unit range covered by a full ±1 mod.
    modKnobs = {
        { &grainSection.grainSize,    "grain_size_ms",   ModTarget::GrainSize,        500.0f,    1.0f,   500.0f  },
        { &grainSection.grainDensity, "grain_density",   ModTarget::GrainDensity,     500.0f,    1.0f,   500.0f  },
        { &playheadSection.position,  "position",        ModTarget::Position,           1.0f,    0.0f,     1.0f  },
        { &playheadSection.spray,     "spray_ms",        ModTarget::Spray,            500.0f,    0.0f,   500.0f  },
        { &playheadSection.scanRate,  "scan_rate_hz",    ModTarget::ScanRate,          10.0f,  0.001f,    10.0f  },
        { &pitchSection.pitchShift,   "pitch_shift_st",  ModTarget::GrainPitch,        48.0f,  -48.0f,    48.0f  },
        { &pitchSection.detune,       "detune_cents",    ModTarget::Detune,           100.0f, -100.0f,   100.0f  },
        { &pitchSection.playbackRate, "playback_rate",   ModTarget::PlaybackRate,       3.99f,  0.01f,     4.0f  },
        { &ampSection.amplitude,      "amplitude",       ModTarget::GrainAmplitude,     1.0f,   0.0f,     1.0f  },
        { &spatialSection.pan,        "pan",             ModTarget::GrainPan,           1.0f,  -1.0f,     1.0f  },
        { &filterSection.cutoff,      "filter_cutoff_hz",ModTarget::FilterCutoff,  20000.0f,   20.0f, 20000.0f  },
        { &filterSection.resonance,   "filter_resonance",ModTarget::FilterResonance,   40.0f,   0.1f,    40.0f  },
    };

    startTimerHz(30);
}

PluginEditor::~PluginEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void PluginEditor::timerCallback()
{
    const int n = processor.getSynth().getTotalActiveGrains();
    grainDotDisplay.setActiveCount(n);

    juce::Array<float> positions;
    const int bufLen = static_cast<int>(processor.getGrainBuffer().getTotalLengthSamples());
    processor.getSynth().collectGrainPositions(positions, bufLen);
    waveformDisplay.setGrainPositions(positions);

    updateModDisplay();
}

void PluginEditor::updateModDisplay()
{
    auto& apvts = processor.getAPVTS();

    for (const auto& b : modKnobs)
    {
        const float mod = processor.modDisplayValues[static_cast<int>(b.target)].load();

        auto* param = dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(b.paramId));
        if (param == nullptr) continue;

        const float base = param->getNormalisableRange().convertFrom0to1(param->getValue());

        if (std::abs(mod) < 0.001f)
        {
            // No active modulation — restore knob to base APVTS value
            b.knob->pushModulatedValue(static_cast<double>(base));
            continue;
        }

        const float modulated = juce::jlimit(b.min, b.max, base + mod * b.scale);
        b.knob->pushModulatedValue(static_cast<double>(modulated));
    }
}

void PluginEditor::mouseDown(const juce::MouseEvent& e)
{
    if (e.eventComponent == &titleLabel)
        randomizeAllParams();
}

void PluginEditor::randomizeAllParams()
{
    // Skip params that damage the mix without contributing to texture
    static const juce::StringArray kSkip {
        "master_volume_db", "dry_wet", "input_gain_db",
        "pan", "pan_random",
        "stereo_width", "voices",
        "record_mode", "record_feedback", "buffer_length_ms",
        "fx_reverb_mix", "fx_delay_mix", "fx_chorus_mix",
        "lfo1_depth", "lfo2_depth", "lfo3_depth", "lfo4_depth",
        "freeze", "dc_filter", "anti_aliasing",
        "filter_cutoff_hz"   // going near 20 Hz silences the output
    };

    // Constrained params: normalized [lo, hi] ranges for musical safety
    struct Constraint { const char* id; float lo, hi; };
    static const Constraint kConstrained[] = {
        { "pitch_shift_st",    0.375f, 0.625f },  // ±12st within the ±48st range
        { "grain_probability", 0.5f,   1.0f   },  // never below 50%: too sparse otherwise
    };

    auto& rng = juce::Random::getSystemRandom();

    for (auto* p : processor.getParameters())
    {
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(p))
        {
            if (kSkip.contains(rp->paramID)) continue;

            bool constrained = false;
            for (const auto& c : kConstrained)
            {
                if (rp->paramID == c.id)
                {
                    rp->setValueNotifyingHost(c.lo + rng.nextFloat() * (c.hi - c.lo));
                    constrained = true;
                    break;
                }
            }
            if (!constrained)
                rp->setValueNotifyingHost(rng.nextFloat());
        }
    }
}

void PluginEditor::paint(juce::Graphics& g)
{
    // Radial gradient background — subtle depth
    {
        juce::ColourGradient grad(juce::Colour(0xff101010),
                                   kW * 0.5f, kH * 0.5f,
                                   juce::Colour(0xff080808),
                                   0.0f, 0.0f,
                                   true);   // radial
        g.setGradientFill(grad);
        g.fillRect(0, 0, kW, kH);
    }

    // Header bar
    g.setColour(GranularLookAndFeel::backgroundMid);
    g.fillRect(0, 0, kW, kHeaderH);

    // Waveform strip bg (cols 1–4 only; LFO col stays dark)
    g.setColour(GranularLookAndFeel::backgroundMid.darker(0.15f));
    g.fillRect(0, kHeaderH, kMainW, kWaveH);

    // LFO column bg
    g.setColour(GranularLookAndFeel::backgroundMid.darker(0.3f));
    g.fillRect(kMainW, 0, kLfoColW, kH);

    const auto hairline = GranularLookAndFeel::textSecondary.withAlpha(0.18f);

    // Section background fills — subtle steps darker left→right
    const int grainH  = static_cast<int>(kBodyH * 0.50f);
    const int pitchH  = kBodyH / 2;
    const int spaceH  = static_cast<int>(kBodyH * 0.22f);
    const int filterH = static_cast<int>(kBodyH * 0.50f);
    const int bufferH = kBodyH - spaceH - filterH;

    struct SectionFill { int x, y, w, h; juce::Colour bg; };
    const SectionFill fills[] = {
        { 0,         kBodyY,                          kColW,    grainH,            juce::Colour(0xff181818) }, // Grain Core
        { 0,         kBodyY + grainH,                 kColW,    kBodyH - grainH,   juce::Colour(0xff161616) }, // Playhead
        { kColW,     kBodyY,                          kColW,    pitchH,            juce::Colour(0xff151515) }, // Pitch
        { kColW,     kBodyY + pitchH,                 kColW,    kBodyH - pitchH,   juce::Colour(0xff131313) }, // Amplitude
        { kColW * 2, kBodyY,                          kColW,    spaceH,            juce::Colour(0xff171717) }, // Spatial
        { kColW * 2, kBodyY + spaceH,                 kColW,    filterH,           juce::Colour(0xff141414) }, // Filter
        { kColW * 2, kBodyY + spaceH + filterH,       kColW,    bufferH,           juce::Colour(0xff121212) }, // Buffer
        { kColW * 3, kBodyY,                          kColW,    kBodyH,            juce::Colour(0xff111111) }, // Effects
        { kMainW,    kBodyY,                          kLfoColW, kBodyH,            juce::Colour(0xff0f0f0f) }, // Modulation
    };
    for (const auto& f : fills)
    {
        g.setColour(f.bg);
        g.fillRect(f.x, f.y, f.w, f.h);
    }

    // Horizontal dividers (drawn on top of fills)
    g.setColour(hairline);
    g.drawHorizontalLine(kHeaderH, 0.0f, static_cast<float>(kW));
    g.drawHorizontalLine(kBodyY,   0.0f, static_cast<float>(kW));

    // Vertical column dividers
    for (int i = 1; i < 4; ++i)
        g.drawVerticalLine(i * kColW, static_cast<float>(kBodyY), static_cast<float>(kH));

    // LFO column left border
    g.drawVerticalLine(kMainW, static_cast<float>(kHeaderH), static_cast<float>(kH));

    // LFO footer divider
    g.drawHorizontalLine(kBodyY + kBodyH - kLfoFootH,
                         static_cast<float>(kMainW), static_cast<float>(kW));

    // Section left-edge bars — 3px, stepped greys (drawn on top of fills)
    struct Bar { int x, y, h; juce::Colour colour; };
    const Bar bars[] = {
        { 0,         kBodyY,                          grainH,            juce::Colour(0xff404040) }, // Grain Core
        { 0,         kBodyY + grainH,                 kBodyH - grainH,   juce::Colour(0xff383838) }, // Playhead
        { kColW,     kBodyY,                          pitchH,            juce::Colour(0xff353535) }, // Pitch
        { kColW,     kBodyY + pitchH,                 kBodyH - pitchH,   juce::Colour(0xff303030) }, // Amplitude
        { kColW * 2, kBodyY,                          spaceH,            juce::Colour(0xff2d2d2d) }, // Spatial
        { kColW * 2, kBodyY + spaceH,                 filterH,           juce::Colour(0xff292929) }, // Filter
        { kColW * 2, kBodyY + spaceH + filterH,       bufferH,           juce::Colour(0xff252525) }, // Buffer
        { kColW * 3, kBodyY,                          kBodyH,            juce::Colour(0xff222222) }, // Effects
        { kMainW,    kBodyY,                          kBodyH,            juce::Colour(0xff1e1e1e) }, // Modulation
    };
    for (const auto& bar : bars)
    {
        g.setColour(bar.colour);
        g.fillRect(bar.x, bar.y, 3, bar.h);
    }
}

void PluginEditor::resized()
{
    const int pad  = 8;
    constexpr int kSLH = 4;

    // Header
    titleLabel.setBounds(16, 0, 250, kHeaderH);
    startBtn.setBounds(278, 8, 62, 28);
    stopBtn.setBounds (344, 8, 62, 28);
    grainDotDisplay.setBounds(kMainW - 58, 0, 50, kHeaderH);

    // Waveform strip (cols 1–4, inset)
    waveformDisplay.setBounds(pad, kHeaderH + pad,
                              kMainW - pad * 2, kWaveH - pad * 2);

    // ── Column 1: Grain + Playhead ──────────────────────────────────────────
    const int c1x = 0;
    const int grainH = static_cast<int>(kBodyH * 0.50f);
    grainSection.setBounds   (c1x + pad,  kBodyY + kSLH,           kColW - pad * 2, grainH - kSLH - pad);
    playheadSection.setBounds(c1x + pad,  kBodyY + grainH + kSLH, kColW - pad * 2, kBodyH - grainH - kSLH - pad);

    // ── Column 2: Pitch + Amplitude ─────────────────────────────────────────
    const int c2x = kColW;
    const int pitchH = kBodyH / 2;
    pitchSection.setBounds(c2x + pad, kBodyY + kSLH,         kColW - pad * 2, pitchH - kSLH - pad);
    ampSection.setBounds(c2x + pad,  kBodyY + pitchH + kSLH, kColW - pad * 2, kBodyH - pitchH - kSLH - pad);

    // ── Column 3: Space + Filter + Buffer ───────────────────────────────────
    const int c3x      = kColW * 2;
    const int spaceH   = static_cast<int>(kBodyH * 0.22f);
    const int filterH  = static_cast<int>(kBodyH * 0.50f);
    const int bufferH  = kBodyH - spaceH - filterH;
    spatialSection.setBounds(c3x + pad, kBodyY + kSLH,                   kColW - pad * 2, spaceH - kSLH - pad);
    filterSection.setBounds(c3x + pad, kBodyY + spaceH + kSLH,           kColW - pad * 2, filterH - kSLH - pad);
    bufferSection.setBounds(c3x + pad, kBodyY + spaceH + filterH + kSLH, kColW - pad * 2, bufferH - kSLH - pad);

    // ── Column 4: FX (full column) ──────────────────────────────────────────
    const int c4x = kColW * 3;
    fxSection.setBounds(c4x + pad, kBodyY + kSLH, kColW - pad * 2, kBodyH - kSLH - pad);

    // ── Column 5: Mod (LFO strips) + output footer ──────────────────────────
    const int modH  = kBodyH - kLfoFootH;
    const int footY = kBodyY + modH;
    modSection.setBounds(kMainW + pad, kBodyY + kSLH, kLfoColW - pad * 2, modH - kSLH - pad);

    // GAIN fader — full footer width
    const int lblH    = 12;
    const int faderH  = 40;
    const int gainW   = kLfoColW - pad * 2;
    const int faderY  = footY + (kLfoFootH - faderH - lblH - 4) / 2;
    masterVolLabel.setBounds (kMainW + pad, faderY,           gainW, lblH);
    masterVolSlider.setBounds(kMainW + pad, faderY + lblH + 4, gainW, faderH);
}
