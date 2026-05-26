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
      recordSection   (p),
      modSection      (p.getAPVTS(), p.getModMatrix()),
      waveformDisplay (p.getFormatManager()),
      masterVolAtt    (p.getAPVTS(), "master_volume_db", masterVolSlider),
      dryWetAtt       (p.getAPVTS(), "dry_wet",          dryWetSlider)
{
    setLookAndFeel(&laf);
    setSize(kW, kH);

    titleLabel.setText("GRANULATOR", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, GranularLookAndFeel::accent);
    titleLabel.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    titleLabel.addMouseListener(this, false);
    addAndMakeVisible(titleLabel);

    grainCountLabel.setFont(juce::Font(10.0f));
    grainCountLabel.setColour(juce::Label::textColourId, GranularLookAndFeel::textSecondary);
    grainCountLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(grainCountLabel);

    // Play toggle — sends a held MIDI note through the synth
    playBtn.setButtonText("PLAY");
    playBtn.setClickingTogglesState(true);
    playBtn.onClick = [this]()
    {
        if (playBtn.getToggleState())
            processor.getSynth().noteOn(1, 60, 0.8f);
        else
            processor.getSynth().noteOff(1, 60, 0.8f, true);
    };
    addAndMakeVisible(playBtn);

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
    addAndMakeVisible(recordSection);
    addAndMakeVisible(modSection);

    masterVolLabel.setText("VOL", juce::dontSendNotification);
    masterVolLabel.setJustificationType(juce::Justification::centred);
    masterVolLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    masterVolLabel.setColour(juce::Label::textColourId, GranularLookAndFeel::textSecondary);

    dryWetLabel.setText("WET", juce::dontSendNotification);
    dryWetLabel.setJustificationType(juce::Justification::centred);
    dryWetLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    dryWetLabel.setColour(juce::Label::textColourId, GranularLookAndFeel::textSecondary);

    addAndMakeVisible(masterVolSlider);
    addAndMakeVisible(masterVolLabel);
    addAndMakeVisible(dryWetSlider);
    addAndMakeVisible(dryWetLabel);

    masterVolSlider.setTextValueSuffix(" dB");
    masterVolSlider.setNumDecimalPlacesToDisplay(1);
    dryWetSlider.setTextValueSuffix("%");
    dryWetSlider.setNumDecimalPlacesToDisplay(1);

    bufferSection.onLoadFile = [&](const juce::File& file) {
        p.getGrainBuffer().loadFile(file, p.getFormatManager());
        waveformDisplay.loadFile(file);
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
    static const juce::StringArray kSkip { "master_volume_db", "dry_wet" };
    auto& rng = juce::Random::getSystemRandom();

    for (auto* p : processor.getParameters())
    {
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(p))
        {
            if (kSkip.contains(rp->paramID)) continue;
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
    const int spaceH  = static_cast<int>(kBodyH * 0.28f);
    const int filterH = static_cast<int>(kBodyH * 0.36f);
    const int bufferH = kBodyH - spaceH - filterH;
    const int fxH     = static_cast<int>(kBodyH * 0.65f);
    const int recH    = kBodyH - fxH;

    struct Bar { int x, y, h; juce::Colour colour; };
    const Bar bars[] = {
        { 0,         kBodyY,                       grainH,          juce::Colour(0xffb08040) }, // Grain Core
        { 0,         kBodyY + grainH,              kBodyH - grainH, juce::Colour(0xff707870) }, // Playhead
        { kColW,     kBodyY,                       pitchH,          juce::Colour(0xffd06030) }, // Pitch
        { kColW,     kBodyY + pitchH,              kBodyH - pitchH, juce::Colour(0xffc05535) }, // Amplitude
        { kColW * 2, kBodyY,                       spaceH,          juce::Colour(0xff4080b0) }, // Spatial
        { kColW * 2, kBodyY + spaceH,              filterH,         juce::Colour(0xff30a0a0) }, // Filter
        { kColW * 2, kBodyY + spaceH + filterH,    bufferH,         juce::Colour(0xff607060) }, // Buffer
        { kColW * 3, kBodyY,                       fxH,             juce::Colour(0xff5060c0) }, // Effects
        { kColW * 3, kBodyY + fxH,                 recH,            juce::Colour(0xffc03030) }, // Record
        { kMainW,    kBodyY,                       kBodyH,          juce::Colour(0xff8050b8) }, // Modulation
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
    titleLabel.setBounds(16, 0, 180, kHeaderH);
    playBtn.setBounds(210, 8, 70, 28);
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
    const int spaceH   = static_cast<int>(kBodyH * 0.28f);
    const int filterH  = static_cast<int>(kBodyH * 0.36f);
    const int bufferH  = kBodyH - spaceH - filterH;
    spatialSection.setBounds(c3x + pad, kBodyY + kSLH,                   kColW - pad * 2, spaceH - kSLH - pad);
    filterSection.setBounds(c3x + pad, kBodyY + spaceH + kSLH,           kColW - pad * 2, filterH - kSLH - pad);
    bufferSection.setBounds(c3x + pad, kBodyY + spaceH + filterH + kSLH, kColW - pad * 2, bufferH - kSLH - pad);

    // ── Column 4: FX + Record ───────────────────────────────────────────────
    const int c4x   = kColW * 3;
    const int fxH   = static_cast<int>(kBodyH * 0.65f);
    const int recH  = kBodyH - fxH;
    fxSection.setBounds(c4x + pad,  kBodyY + kSLH,      kColW - pad * 2, fxH - kSLH - pad);
    recordSection.setBounds(c4x + pad, kBodyY + fxH + kSLH, kColW - pad * 2, recH - kSLH - pad);

    // ── Column 5: Mod (LFO strips) + output footer ──────────────────────────
    const int modH  = kBodyH - kLfoFootH;
    const int footY = kBodyY + modH;
    modSection.setBounds(kMainW + pad, kBodyY + kSLH, kLfoColW - pad * 2, modH - kSLH - pad);

    // VOL + WET side by side in LFO footer
    const int knobW = (kLfoColW - pad * 3) / 2;
    const int lblH  = 12;
    const int knobSz = kLfoFootH - lblH - pad;
    masterVolLabel.setBounds (kMainW + pad,            footY + pad,        knobW, lblH);
    masterVolSlider.setBounds(kMainW + pad,            footY + pad + lblH, knobW, knobSz);
    dryWetLabel.setBounds    (kMainW + pad * 2 + knobW, footY + pad,       knobW, lblH);
    dryWetSlider.setBounds   (kMainW + pad * 2 + knobW, footY + pad + lblH, knobW, knobSz);
}
