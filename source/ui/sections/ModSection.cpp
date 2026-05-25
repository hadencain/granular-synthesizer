#include "ModSection.h"

ModSection::LFOStrip::LFOStrip(juce::AudioProcessorValueTreeState& apvts, int n)
{
    auto ns = juce::String(n);
    label.setText("LFO " + ns, juce::dontSendNotification);
    shapeLabel.setText("Shape", juce::dontSendNotification);
    rate.setLabel("Rate");
    depth.setLabel("Depth");
    phase.setLabel("Phase");

    rateAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "lfo" + ns + "_rate_hz",  rate.getSlider());
    depthAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "lfo" + ns + "_depth",    depth.getSlider());
    phaseAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "lfo" + ns + "_phase_deg",phase.getSlider());
    shapeBox.addItemList({"Sine","Triangle","Saw Up","Saw Down","Square","S&H","Smooth Rnd"}, 1);
    shapeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "lfo" + ns + "_shape", shapeBox);

    addAndMakeVisible(label);
    addAndMakeVisible(shapeLabel);
    addAndMakeVisible(shapeBox);
    addAndMakeVisible(rate);
    addAndMakeVisible(depth);
    addAndMakeVisible(phase);
}

void ModSection::LFOStrip::resized()
{
    const int h = getHeight();
    const int knobW = 60;
    int x = 0;
    label.setBounds(x, 0, 40, h); x += 45;
    shapeLabel.setBounds(x, 0, 50, 14);
    shapeBox.setBounds(x, 16, 80, 20); x += 90;
    rate.setBounds (x, 0, knobW, h); x += knobW + 4;
    depth.setBounds(x, 0, knobW, h); x += knobW + 4;
    phase.setBounds(x, 0, knobW, h);
}

ModSection::ModSection(juce::AudioProcessorValueTreeState& apvts, ModMatrix& modMatrix)
    : modGrid(modMatrix)
{
    for (int i = 0; i < 4; ++i)
    {
        lfoStrips[i] = std::make_unique<LFOStrip>(apvts, i + 1);
        addAndMakeVisible(*lfoStrips[i]);
    }
    addAndMakeVisible(modGrid);
}

void ModSection::resized()
{
    const int stripH = 52;
    const int gap    = 4;
    int y = 4;
    for (auto& strip : lfoStrips)
    {
        strip->setBounds(4, y, getWidth() - 8, stripH);
        y += stripH + gap;
    }
    modGrid.setBounds(4, y, getWidth() - 8, getHeight() - y - 4);
}
