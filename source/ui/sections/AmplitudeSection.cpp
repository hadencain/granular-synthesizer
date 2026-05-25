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

void AmplitudeSection::resized()
{
    const int knobW = 80, knobH = 90, gap = 10;
    int x = gap;
    for (auto* c : { &amplitude, &ampRandom, &velSens, &crossfade })
    { c->setBounds(x, 10, knobW, knobH); x += knobW + gap; }

    x = gap;
    for (auto* c : { &attack, &decay, &sustain, &release })
    { c->setBounds(x, 10 + knobH + gap, knobW, knobH); x += knobW + gap; }
}
