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
    const int knobW  = 80;
    const int knobH  = 90;
    const int gap    = 10;
    const int row1Y  = 10;
    const int row2Y  = row1Y + knobH + gap;
    const int comboH = 22;

    int x = gap;
    for (auto* c : { &grainSize, &grainDensity, &grainOverlap, &interonset })
    {
        c->setBounds(x, row1Y, knobW, knobH);
        x += knobW + gap;
    }

    x = gap;
    for (auto* c : { &randomizeSize, &randomizeDensity })
    {
        c->setBounds(x, row2Y, knobW, knobH);
        x += knobW + gap;
    }

    // Envelope and Direction combo boxes in row 2, after random knobs
    envelopeLabel.setBounds(x, row2Y, knobW, 14);
    envelopeBox.setBounds(x, row2Y + 16, knobW + 20, comboH);
    x += knobW + 30;

    directionLabel.setBounds(x, row2Y, knobW, 14);
    directionBox.setBounds(x, row2Y + 16, knobW + 20, comboH);
}
