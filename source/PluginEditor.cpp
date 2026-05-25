#include "PluginEditor.h"

static constexpr int kHeaderH = 52;
static constexpr int kSideW   = 240;

PluginEditor::PluginEditor(PluginProcessor& p)
    : AudioProcessorEditor(&p),
      processor(p),
      grainSection    (p.getAPVTS()),
      playheadSection (p.getAPVTS()),
      pitchSection    (p.getAPVTS()),
      ampSection      (p.getAPVTS()),
      spatialSection  (p.getAPVTS()),
      modSection      (p.getAPVTS(), p.getModMatrix()),
      filterSection   (p.getAPVTS()),
      bufferSection   (p.getAPVTS()),
      fxSection       (p.getAPVTS()),
      recordSection   (p),
      waveformDisplay (p.getFormatManager()),
      masterVolAtt    (p.getAPVTS(), "master_volume_db", masterVolSlider),
      dryWetAtt       (p.getAPVTS(), "dry_wet",          dryWetSlider)
{
    setLookAndFeel(&laf);
    setSize(900, 620);

    // Header
    titleLabel.setText("GRANULATOR", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(22.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, GranularLookAndFeel::accent);
    addAndMakeVisible(titleLabel);

    grainCountLabel.setFont(juce::Font(10.0f));
    grainCountLabel.setColour(juce::Label::textColourId, GranularLookAndFeel::textSecondary);
    grainCountLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(grainCountLabel);

    // Tabs
    const juce::Colour tabBg = GranularLookAndFeel::backgroundMid;
    tabs.addTab("GRAIN",   tabBg, &grainSection,    false);
    tabs.addTab("PLAY",    tabBg, &playheadSection,  false);
    tabs.addTab("PITCH",   tabBg, &pitchSection,     false);
    tabs.addTab("AMP",     tabBg, &ampSection,       false);
    tabs.addTab("SPACE",   tabBg, &spatialSection,   false);
    tabs.addTab("MOD",     tabBg, &modSection,       false);
    tabs.addTab("FILTER",  tabBg, &filterSection,    false);
    tabs.addTab("BUFFER",  tabBg, &bufferSection,    false);
    tabs.addTab("FX",      tabBg, &fxSection,        false);
    tabs.addTab("REC",     tabBg, &recordSection,    false);
    tabs.setTabBarDepth(32);
    addAndMakeVisible(tabs);

    // Side panel
    waveformDisplay.setBuffer(&p.getGrainBuffer());
    addAndMakeVisible(waveformDisplay);

    masterVolLabel.setText("VOL", juce::dontSendNotification);
    masterVolLabel.setJustificationType(juce::Justification::centred);
    masterVolLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    masterVolLabel.setColour(juce::Label::textColourId, GranularLookAndFeel::textSecondary);

    dryWetLabel.setText("WET", juce::dontSendNotification);
    dryWetLabel.setJustificationType(juce::Justification::centred);
    dryWetLabel.setFont(juce::Font(10.0f, juce::Font::bold));
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

void PluginEditor::paint(juce::Graphics& g)
{
    // Base fill
    g.fillAll(GranularLookAndFeel::backgroundDark);

    // Header bar
    g.setColour(GranularLookAndFeel::backgroundMid);
    g.fillRect(0, 0, getWidth(), kHeaderH);

    // Hairline below header
    g.setColour(GranularLookAndFeel::textSecondary.withAlpha(0.3f));
    g.drawHorizontalLine(kHeaderH, 0.0f, static_cast<float>(getWidth()));

    // Side panel surface
    const int sideX = getWidth() - kSideW;
    g.setColour(GranularLookAndFeel::backgroundMid);
    g.fillRect(sideX, kHeaderH, kSideW, getHeight() - kHeaderH);

    // Side panel left border
    g.setColour(GranularLookAndFeel::textSecondary.withAlpha(0.2f));
    g.drawVerticalLine(sideX, static_cast<float>(kHeaderH), static_cast<float>(getHeight()));
}

void PluginEditor::resized()
{
    const int mainW = getWidth() - kSideW;
    const int sideX = mainW;

    titleLabel.setBounds(18, 0, 220, kHeaderH);
    grainCountLabel.setBounds(mainW - 140, 0, 130, kHeaderH);

    tabs.setBounds(0, kHeaderH, mainW, getHeight() - kHeaderH);

    // Side panel contents
    const int pad  = 12;
    const int wfH  = 148;
    waveformDisplay.setBounds(sideX + pad, kHeaderH + pad,
                               kSideW - pad * 2, wfH);

    const int ctrlY  = kHeaderH + pad + wfH + pad;
    const int knobSz = 72;
    const int lblH   = 14;

    masterVolLabel.setBounds(sideX + pad, ctrlY, knobSz, lblH);
    masterVolSlider.setBounds(sideX + pad, ctrlY + lblH, knobSz, knobSz + 18);

    const int col2X = sideX + pad + knobSz + 12;
    dryWetLabel.setBounds(col2X, ctrlY, knobSz, lblH);
    dryWetSlider.setBounds(col2X, ctrlY + lblH, knobSz, knobSz + 18);
}
