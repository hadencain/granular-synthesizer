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

    amplitude.getSlider().textFromValueFunction = [](double v) -> juce::String {
        if (v <= 0.0001) return "-inf dB";
        return juce::String(20.0 * std::log10(v), 1) + " dB";
    };
    amplitude.getSlider().valueFromTextFunction = [](const juce::String& t) -> double {
        auto s = t.upToFirstOccurrenceOf(" dB", false, true).trim();
        if (s.containsIgnoreCase("inf")) return 0.0;
        return std::pow(10.0, s.getDoubleValue() / 20.0);
    };

    ampRandom.getSlider().setTextValueSuffix(" dB");
    ampRandom.getSlider().setNumDecimalPlacesToDisplay(1);

    velSens.getSlider().textFromValueFunction = [](double v) -> juce::String {
        return juce::String(juce::roundToInt(v * 100)) + "%";
    };
    velSens.getSlider().valueFromTextFunction = [](const juce::String& t) -> double {
        return t.trimCharactersAtEnd("%").getDoubleValue() / 100.0;
    };

    crossfade.getSlider().setTextValueSuffix(" ms");
    crossfade.getSlider().setNumDecimalPlacesToDisplay(1);

    attack.getSlider().setTextValueSuffix(" ms");
    attack.getSlider().setNumDecimalPlacesToDisplay(0);

    decay.getSlider().setTextValueSuffix(" ms");
    decay.getSlider().setNumDecimalPlacesToDisplay(0);

    sustain.getSlider().textFromValueFunction = [](double v) -> juce::String {
        return juce::String(juce::roundToInt(v * 100)) + "%";
    };
    sustain.getSlider().valueFromTextFunction = [](const juce::String& t) -> double {
        return t.trimCharactersAtEnd("%").getDoubleValue() / 100.0;
    };

    release.getSlider().setTextValueSuffix(" ms");
    release.getSlider().setNumDecimalPlacesToDisplay(0);

    for (auto* c : { &amplitude, &ampRandom, &velSens, &crossfade,
                     &attack, &decay, &sustain, &release })
        addAndMakeVisible(c);
}

void AmplitudeSection::resized()
{
    const int kW = 70, kH = 82, gap = 8, startX = 8, startY = 8;
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
