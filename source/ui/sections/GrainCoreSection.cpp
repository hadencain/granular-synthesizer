#include "GrainCoreSection.h"

GrainCoreSection::GrainCoreSection(juce::AudioProcessorValueTreeState& apvts)
    : sizeAtt      (apvts, "grain_size_ms",    grainSize.getSlider()),
      densityAtt   (apvts, "grain_density",    grainDensity.getSlider()),
      overlapAtt   (apvts, "grain_overlap",    grainOverlap.getSlider()),
      interonsetAtt(apvts, "interonset_ms",    interonset.getSlider()),
      randSizeAtt  (apvts, "randomize_size_ms",randomizeSize.getSlider()),
      randDensityAtt(apvts,"randomize_density",randomizeDensity.getSlider())
{
    grainSize.setLabel("Size");
    grainDensity.setLabel("Density");
    grainOverlap.setLabel("Overlap");
    interonset.setLabel("Interonset");
    randomizeSize.setLabel("Rnd Size");
    randomizeDensity.setLabel("Rnd Density");

    grainSize.getSlider().setTextValueSuffix(" ms");
    grainSize.getSlider().setNumDecimalPlacesToDisplay(1);

    grainDensity.getSlider().setTextValueSuffix(" /s");
    grainDensity.getSlider().setNumDecimalPlacesToDisplay(1);

    grainOverlap.getSlider().setTextValueSuffix("%");
    grainOverlap.getSlider().setNumDecimalPlacesToDisplay(1);

    interonset.getSlider().setTextValueSuffix(" ms");
    interonset.getSlider().setNumDecimalPlacesToDisplay(1);

    randomizeSize.getSlider().setTextValueSuffix("%");
    randomizeSize.getSlider().setNumDecimalPlacesToDisplay(1);

    randomizeDensity.getSlider().setTextValueSuffix("%");
    randomizeDensity.getSlider().setNumDecimalPlacesToDisplay(1);

    envelopeLabel.setText("Envelope", juce::dontSendNotification);
    directionLabel.setText("Direction", juce::dontSendNotification);

    envelopeBox.addItemList({"Hann","Gaussian","Trapezoid","Rectangle","Triangular","Tukey"}, 1);
    directionBox.addItemList({"Forward","Reverse","Bidirectional","Random"}, 1);
    envAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "grain_envelope", envelopeBox);
    dirAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "grain_direction", directionBox);

    for (auto* c : { &grainSize, &grainDensity, &grainOverlap,
                     &interonset, &randomizeSize, &randomizeDensity })
        addAndMakeVisible(c);

    addAndMakeVisible(envelopeBox);
    addAndMakeVisible(directionBox);
    addAndMakeVisible(envelopeLabel);
    addAndMakeVisible(directionLabel);
}

void GrainCoreSection::resized()
{
    const int kW = 70, kH = 82, gap = 8, startX = 8, startY = 8;
    const int row2Y = startY + kH + gap;
    const int row3Y = row2Y + kH + gap;

    int x = startX;
    for (auto* c : { &grainSize, &grainDensity, &grainOverlap })
    { c->setBounds(x, startY, kW, kH); x += kW + gap; }

    x = startX;
    for (auto* c : { &interonset, &randomizeSize, &randomizeDensity })
    { c->setBounds(x, row2Y, kW, kH); x += kW + gap; }

    const int comboW = 110, comboH = 22;
    x = startX;
    envelopeLabel.setBounds(x, row3Y, comboW, 14);
    envelopeBox.setBounds(x, row3Y + 16, comboW, comboH);
    x += comboW + gap;
    directionLabel.setBounds(x, row3Y, comboW, 14);
    directionBox.setBounds(x, row3Y + 16, comboW, comboH);
}
