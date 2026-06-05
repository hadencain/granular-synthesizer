#include "AmplitudeSection.h"

AmplitudeSection::AmplitudeSection(juce::AudioProcessorValueTreeState& apvts)
    : ampAtt  (apvts, "amplitude",           amplitude.getSlider()),
      randAtt (apvts, "amp_random_db",       ampRandom.getSlider()),
      velAtt  (apvts, "velocity_sensitivity",velSens.getSlider()),
      xfadeAtt(apvts, "crossfade_ms",        crossfade.getSlider()),
      atkAtt  (apvts, "adsr_attack_ms",      attack.getSlider()),
      decAtt  (apvts, "adsr_decay_ms",       decay.getSlider()),
      susAtt  (apvts, "adsr_sustain",        sustain.getSlider()),
      relAtt  (apvts, "adsr_release_ms",     release.getSlider())
{
    amplitude.setLabel("Amplitude");
    ampRandom.setLabel("Amp Rnd");
    velSens.setLabel("Vel Sens");
    crossfade.setLabel("Crossfade");
    attack.setLabel("Attack");
    decay.setLabel("Decay");
    sustain.setLabel("Sustain");
    release.setLabel("Release");

    for (auto* c : { &amplitude, &ampRandom, &velSens, &crossfade,
                     &attack, &decay, &sustain, &release })
        addAndMakeVisible(c);
}

void AmplitudeSection::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff555555));
    g.setFont(juce::Font(7.5f, juce::Font::bold));
    g.drawText("AMPLITUDE", 8, 2, getWidth() - 16, 11, juce::Justification::centredLeft);
}

void AmplitudeSection::resized()
{
    const int kW = 70, kH = 82, gap = 8, startX = 8, startY = 18;
    const int row2Y = startY + kH + gap;
    const int row3Y = row2Y + kH + gap;

    int x = startX;
    for (auto* c : { &amplitude, &ampRandom, &velSens })
    { c->setBounds(x, startY, kW, kH); x += kW + gap; }

    x = startX;
    for (auto* c : { &crossfade, &attack, &decay })
    { c->setBounds(x, row2Y, kW, kH); x += kW + gap; }

    x = startX;
    for (auto* c : { &sustain, &release })
    { c->setBounds(x, row3Y, kW, kH); x += kW + gap; }
}
