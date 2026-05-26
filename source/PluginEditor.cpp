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
    g.fillAll(GranularLookAndFeel::backgroundDark);

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
}

void PluginEditor::resized()
{
    const int pad = 8;

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
    grainSection.setBounds   (c1x + pad, kBodyY + pad, kColW - pad * 2, grainH - pad);
    playheadSection.setBounds(c1x + pad, kBodyY + grainH, kColW - pad * 2, kBodyH - grainH - pad);

    // ── Column 2: Pitch + Amplitude ─────────────────────────────────────────
    const int c2x = kColW;
    const int pitchH = kBodyH / 2;
    pitchSection.setBounds(c2x + pad, kBodyY + pad, kColW - pad * 2, pitchH - pad);
    ampSection.setBounds  (c2x + pad, kBodyY + pitchH, kColW - pad * 2, kBodyH - pitchH - pad);

    // ── Column 3: Space + Filter + Buffer ───────────────────────────────────
    const int c3x      = kColW * 2;
    const int spaceH   = static_cast<int>(kBodyH * 0.28f);
    const int filterH  = static_cast<int>(kBodyH * 0.36f);
    const int bufferH  = kBodyH - spaceH - filterH;
    spatialSection.setBounds(c3x + pad, kBodyY + pad,              kColW - pad * 2, spaceH - pad);
    filterSection.setBounds (c3x + pad, kBodyY + spaceH,           kColW - pad * 2, filterH - pad);
    bufferSection.setBounds (c3x + pad, kBodyY + spaceH + filterH, kColW - pad * 2, bufferH - pad);

    // ── Column 4: FX + Record ───────────────────────────────────────────────
    const int c4x   = kColW * 3;
    const int fxH   = static_cast<int>(kBodyH * 0.65f);
    const int recH  = kBodyH - fxH;
    fxSection.setBounds    (c4x + pad, kBodyY + pad, kColW - pad * 2, fxH - pad);
    recordSection.setBounds(c4x + pad, kBodyY + fxH, kColW - pad * 2, recH - pad);

    // ── Column 5: Mod (LFO strips) + output footer ──────────────────────────
    const int modH    = kBodyH - kLfoFootH;
    const int footY   = kBodyY + modH;
    modSection.setBounds(kMainW + pad, kBodyY, kLfoColW - pad * 2, modH - pad);

    // VOL + WET side by side in LFO footer
    const int knobW   = (kLfoColW - pad * 3) / 2;
    const int lblH    = 12;
    const int knobSz  = kLfoFootH - lblH - pad;
    masterVolLabel.setBounds(kMainW + pad,           footY + pad, knobW, lblH);
    masterVolSlider.setBounds(kMainW + pad,          footY + pad + lblH, knobW, knobSz);
    dryWetLabel.setBounds(kMainW + pad * 2 + knobW, footY + pad, knobW, lblH);
    dryWetSlider.setBounds(kMainW + pad * 2 + knobW, footY + pad + lblH, knobW, knobSz);
}
