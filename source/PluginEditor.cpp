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
      masterVolAtt    (p.getAPVTS(), "master_volume_db", masterVolSlider),
      dryWetAtt       (p.getAPVTS(), "dry_wet",          dryWetSlider)
{
    setLookAndFeel(&laf);
    setSize(kW, kH);

    titleLabel.setText("granular synthesizer", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, GranularLookAndFeel::accent);
    titleLabel.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    titleLabel.addMouseListener(this, false);
    addAndMakeVisible(titleLabel);

    grainCountLabel.setFont(juce::Font(10.0f));
    grainCountLabel.setColour(juce::Label::textColourId, GranularLookAndFeel::textSecondary);
    grainCountLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(grainCountLabel);

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

    dryWetLabel.setText("WET", juce::dontSendNotification);
    dryWetLabel.setJustificationType(juce::Justification::centred);
    dryWetLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    dryWetLabel.setColour(juce::Label::textColourId, GranularLookAndFeel::textSecondary);

    masterVolSlider.setTextValueSuffix(" dB");
    masterVolSlider.setNumDecimalPlacesToDisplay(1);
    dryWetSlider.textFromValueFunction = [](double v) -> juce::String {
        return juce::String(juce::roundToInt(v * 100)) + "%";
    };
    dryWetSlider.valueFromTextFunction = [](const juce::String& t) -> double {
        return t.trimCharactersAtEnd("%").getDoubleValue() / 100.0;
    };

    addAndMakeVisible(masterVolSlider);
    addAndMakeVisible(masterVolLabel);
    addAndMakeVisible(dryWetSlider);
    addAndMakeVisible(dryWetLabel);

    waveformDisplay.onLoadFile = [&](const juce::File& file) {
        p.getGrainBuffer().loadFile(file, p.getFormatManager());
    };

    startTimerHz(10);
}

PluginEditor::~PluginEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void PluginEditor::timerCallback()
{
    const int n = processor.getSynth().getTotalActiveGrains();
    grainCountLabel.setText("grains  " + juce::String(n),
                             juce::dontSendNotification);

    juce::Array<float> positions;
    const int bufLen = static_cast<int>(processor.getGrainBuffer().getTotalLengthSamples());
    processor.getSynth().collectGrainPositions(positions, bufLen);
    waveformDisplay.setGrainPositions(positions);
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
        "spatial_width", "spatial_stereo", "voices",
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

    // Horizontal dividers
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

    // Section color bars — 2px left edge of each section
    const int grainH  = static_cast<int>(kBodyH * 0.50f);
    const int pitchH  = kBodyH / 2;
    const int spaceH  = static_cast<int>(kBodyH * 0.22f);
    const int filterH = static_cast<int>(kBodyH * 0.50f);
    const int bufferH = kBodyH - spaceH - filterH;
    const int fxH     = kBodyH;

    struct Bar { int x, y, h; juce::Colour colour; };
    const Bar bars[] = {
        { 0,         kBodyY,                       grainH,          juce::Colour(0xff3c3c3c) }, // Grain Core
        { 0,         kBodyY + grainH,              kBodyH - grainH, juce::Colour(0xff363636) }, // Playhead
        { kColW,     kBodyY,                       pitchH,          juce::Colour(0xff323232) }, // Pitch
        { kColW,     kBodyY + pitchH,              kBodyH - pitchH, juce::Colour(0xff2e2e2e) }, // Amplitude
        { kColW * 2, kBodyY,                       spaceH,          juce::Colour(0xff2a2a2a) }, // Spatial
        { kColW * 2, kBodyY + spaceH,              filterH,         juce::Colour(0xff262626) }, // Filter
        { kColW * 2, kBodyY + spaceH + filterH,    bufferH,         juce::Colour(0xff232323) }, // Buffer
        { kColW * 3, kBodyY,                       fxH,             juce::Colour(0xff202020) }, // Effects
        { kMainW,    kBodyY,                       kBodyH,          juce::Colour(0xff1e1e1e) }, // Modulation
    };
    for (const auto& bar : bars)
    {
        g.setColour(bar.colour);
        g.fillRect(bar.x, bar.y, 2, bar.h);
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
    grainCountLabel.setBounds(kMainW - 130, 0, 120, kHeaderH);

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

    // GAIN slider (narrow vertical) + WET knob in LFO footer
    const int gainW   = 36;
    const int wetW    = 100;
    const int lblH    = 12;
    const int sliderH = kLfoFootH - lblH - pad;
    masterVolLabel.setBounds(kMainW + pad,               footY + pad,        gainW, lblH);
    masterVolSlider.setBounds(kMainW + pad,              footY + pad + lblH, gainW, sliderH);
    dryWetLabel.setBounds   (kMainW + pad * 2 + gainW,   footY + pad,        wetW,  lblH);
    dryWetSlider.setBounds  (kMainW + pad * 2 + gainW,   footY + pad + lblH, wetW,  sliderH);
}
