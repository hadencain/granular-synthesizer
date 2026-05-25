#include "PitchSection.h"

PitchSection::PitchSection(juce::AudioProcessorValueTreeState& apvts)
    : pitchAtt  (apvts, "pitch_shift_st",   pitchShift.getSlider()),
      detuneAtt (apvts, "detune_cents",      detune.getSlider()),
      randAtt   (apvts, "pitch_random_st",  pitchRandom.getSlider()),
      rateAtt   (apvts, "playback_rate",    playbackRate.getSlider()),
      formantAtt(apvts, "formant_shift_st", formantShift.getSlider()),
      glideAtt  (apvts, "glide_ms",         glide.getSlider()),
      transposeAtt(apvts, "transpose_mode", transposeBox),
      quantizeAtt (apvts, "pitch_quantize", quantizeBox)
{
    pitchShift.setLabel("Pitch");
    detune.setLabel("Detune");
    pitchRandom.setLabel("Rnd Pitch");
    playbackRate.setLabel("Rate");
    formantShift.setLabel("Formant");
    glide.setLabel("Glide");
    transposeLabel.setText("Transpose", juce::dontSendNotification);
    quantizeLabel.setText("Quantize", juce::dontSendNotification);

    for (auto* c : { &pitchShift, &detune, &pitchRandom, &playbackRate, &formantShift, &glide })
        addAndMakeVisible(c);
    addAndMakeVisible(transposeBox);  addAndMakeVisible(transposeLabel);
    addAndMakeVisible(quantizeBox);   addAndMakeVisible(quantizeLabel);
}

void PitchSection::resized()
{
    const int knobW = 80, knobH = 90, gap = 10;
    int x = gap;
    for (auto* c : { &pitchShift, &detune, &pitchRandom, &playbackRate })
    { c->setBounds(x, 10, knobW, knobH); x += knobW + gap; }

    x = gap;
    for (auto* c : { &formantShift, &glide })
    { c->setBounds(x, 10 + knobH + gap, knobW, knobH); x += knobW + gap; }

    transposeLabel.setBounds(x, 10 + knobH + gap, 100, 14);
    transposeBox.setBounds  (x, 10 + knobH + gap + 16, 100, 22);
    x += 110;
    quantizeLabel.setBounds (x, 10 + knobH + gap, 100, 14);
    quantizeBox.setBounds   (x, 10 + knobH + gap + 16, 100, 22);
}
