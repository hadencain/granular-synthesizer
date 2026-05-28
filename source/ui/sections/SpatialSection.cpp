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

    voicesSlider.onValueChange = [this] { repaint(); };
}

void SpatialSection::resized()
{
    const int kW = 70, kH = 82, gap = 8, startX = 8, startY = 8;
    int x = startX;
    for (auto* c : { &pan, &panRandom, &stereoWidth })
    { c->setBounds(x, startY, kW, kH); x += kW + gap; }

    // voiceDetune on second row with voices badge
    const int row2Y = startY + kH + gap;
    voiceDetune.setBounds(startX, row2Y, kW, kH);
    voicesSlider.setBounds(startX + kW + gap, row2Y + 30, 80, 22);
}

void SpatialSection::paintOverChildren(juce::Graphics& g)
{
    const auto b = voicesSlider.getBounds().toFloat();
    g.setColour(juce::Colour(0xff141414));
    g.fillRoundedRectangle(b, 3.0f);
    g.setColour(juce::Colour(0xff909090));
    g.drawRoundedRectangle(b, 3.0f, 1.0f);
    g.setColour(juce::Colour(0xffd0d0d0));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::bold));
    g.drawText("VOICES  " + juce::String(static_cast<int>(voicesSlider.getValue())),
               voicesSlider.getBounds(), juce::Justification::centred);
}
