#include "PitchSection.h"

PitchSection::PitchSection(juce::AudioProcessorValueTreeState& apvts)
    : pitchAtt  (apvts, "pitch_shift_st",   pitchShift.getSlider()),
      detuneAtt (apvts, "detune_cents",      detune.getSlider()),
      randAtt   (apvts, "pitch_random_st",  pitchRandom.getSlider()),
      rateAtt   (apvts, "playback_rate",    playbackRate.getSlider()),
      formantAtt(apvts, "formant_shift_st", formantShift.getSlider()),
      glideAtt  (apvts, "glide_ms",         glide.getSlider())
{
    pitchShift.setLabel("Pitch");
    detune.setLabel("Detune");
    pitchRandom.setLabel("Rnd Pitch");
    playbackRate.setLabel("Rate");
    formantShift.setLabel("Formant");
    glide.setLabel("Glide");

    transposeLabel.setText("Transpose", juce::dontSendNotification);
    quantizeLabel.setText("Quantize", juce::dontSendNotification);

    transposeBox.addItemList({"Doppler", "Interpolated", "Phase Vocoder"}, 1);
    quantizeBox.addItemList({"Off", "Chromatic", "Major", "Minor", "Custom"}, 1);
    transposeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "transpose_mode", transposeBox);
    quantizeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "pitch_quantize", quantizeBox);

    for (auto* c : { &pitchShift, &detune, &pitchRandom, &playbackRate, &formantShift, &glide })
        addAndMakeVisible(c);
    addAndMakeVisible(transposeBox);  addAndMakeVisible(transposeLabel);
    addAndMakeVisible(quantizeBox);   addAndMakeVisible(quantizeLabel);
}

void PitchSection::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff555555));
    g.setFont(juce::Font(7.5f, juce::Font::bold));
    g.drawText("PITCH", 8, 2, getWidth() - 16, 11, juce::Justification::centredLeft);
}

void PitchSection::resized()
{
    const int kW = 70, kH = 82, gap = 8, startX = 8, startY = 18;
    const int row2Y = startY + kH + gap;
    const int row3Y = row2Y + kH + gap;

    int x = startX;
    for (auto* c : { &pitchShift, &detune, &pitchRandom })
    { c->setBounds(x, startY, kW, kH); x += kW + gap; }

    x = startX;
    for (auto* c : { &playbackRate, &formantShift, &glide })
    { c->setBounds(x, row2Y, kW, kH); x += kW + gap; }

    const int comboW = 110, comboH = 22;
    x = startX;
    transposeLabel.setBounds(x, row3Y, comboW, 14);
    transposeBox.setBounds(x, row3Y + 16, comboW, comboH);
    x += comboW + gap;
    quantizeLabel.setBounds(x, row3Y, comboW, 14);
    quantizeBox.setBounds(x, row3Y + 16, comboW, comboH);
}
