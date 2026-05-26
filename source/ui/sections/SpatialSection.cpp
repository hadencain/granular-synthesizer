#include "SpatialSection.h"

SpatialSection::SpatialSection(juce::AudioProcessorValueTreeState& apvts)
    : panAtt    (apvts, "pan",               pan.getSlider()),
      panRandAtt(apvts, "pan_random",        panRandom.getSlider()),
      widthAtt  (apvts, "stereo_width",      stereoWidth.getSlider()),
      detuneAtt (apvts, "voice_detune_cents",voiceDetune.getSlider()),
      voicesAtt (apvts, "voices",            voicesSlider)
{
    pan.setLabel("Pan");
    panRandom.setLabel("Pan Rnd");
    stereoWidth.setLabel("Width");
    voiceDetune.setLabel("V.Detune");
    voicesLabel.setText("Voices", juce::dontSendNotification);

    for (auto* c : { &pan, &panRandom, &stereoWidth, &voiceDetune })
        addAndMakeVisible(c);
    addAndMakeVisible(voicesSlider);
    addAndMakeVisible(voicesLabel);
}

void SpatialSection::resized()
{
    const int kW = 70, kH = 82, gap = 8, startX = 8, startY = 8;
    int x = startX;
    for (auto* c : { &pan, &panRandom, &stereoWidth })
    { c->setBounds(x, startY, kW, kH); x += kW + gap; }

    // voiceDetune on second row with voices slider
    const int row2Y = startY + kH + gap;
    voiceDetune.setBounds(startX, row2Y, kW, kH);
    voicesLabel.setBounds(startX + kW + gap, row2Y, 60, 16);
    voicesSlider.setBounds(startX + kW + gap, row2Y + 18, 160, 28);
}
