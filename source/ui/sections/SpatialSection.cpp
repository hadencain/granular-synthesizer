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
    const int knobW = 80, knobH = 90, gap = 10;
    int x = gap;
    for (auto* c : { &pan, &panRandom, &stereoWidth, &voiceDetune })
    { c->setBounds(x, 10, knobW, knobH); x += knobW + gap; }

    voicesLabel.setBounds(gap, 10 + knobH + gap, 60, 16);
    voicesSlider.setBounds(gap, 10 + knobH + gap + 18, 300, 24);
}
