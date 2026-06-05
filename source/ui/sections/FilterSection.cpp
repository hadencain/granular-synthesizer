#include "FilterSection.h"

FilterSection::FilterSection(juce::AudioProcessorValueTreeState& apvts)
    : curveDisplay(apvts),
      cutoffAtt  (apvts, "filter_cutoff_hz",  cutoff.getSlider()),
      resAtt     (apvts, "filter_resonance",   resonance.getSlider()),
      envDepthAtt(apvts, "filter_env_depth",   envDepth.getSlider()),
      lfoDepthAtt(apvts, "filter_lfo_depth",   lfoDepth.getSlider()),
      keytrackAtt(apvts, "filter_keytrack",    keytrack.getSlider())
{
    cutoff.setLabel("Cutoff");
    resonance.setLabel("Resonance");
    envDepth.setLabel("Env Depth");
    lfoDepth.setLabel("LFO Depth");
    keytrack.setLabel("Keytrack");
    filterTypeLabel.setText("Type", juce::dontSendNotification);

    filterTypeBox.addItemList({"Low-Pass", "High-Pass", "Band-Pass", "Notch"}, 1);
    typeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "filter_type", filterTypeBox);

    addAndMakeVisible(curveDisplay);
    for (auto* c : { &cutoff, &resonance, &envDepth, &lfoDepth, &keytrack })
        addAndMakeVisible(c);
    addAndMakeVisible(filterTypeBox);
    addAndMakeVisible(filterTypeLabel);
}

void FilterSection::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff555555));
    g.setFont(juce::Font(7.5f, juce::Font::bold));
    g.drawText("FILTER", 8, 2, getWidth() - 16, 11, juce::Justification::centredLeft);
}

void FilterSection::resized()
{
    const int kW = 70, kH = 72, gap = 6, startX = 8, startY = 16;
    const int fullW = getWidth() - startX * 2;

    // Frequency response curve
    curveDisplay.setBounds(startX, startY, fullW, 58);

    // Type label + combo — compact row below curve
    const int typeY = startY + 58 + gap;
    filterTypeLabel.setBounds(startX, typeY, 50, 12);
    filterTypeBox.setBounds(startX + 52, typeY - 2, 110, 20);

    // Knob rows
    const int row2Y = typeY + 22 + gap;
    int x = startX;
    for (auto* c : { &cutoff, &resonance, &envDepth })
    { c->setBounds(x, row2Y, kW, kH); x += kW + gap; }

    x = startX;
    const int row3Y = row2Y + kH + gap;
    for (auto* c : { &lfoDepth, &keytrack })
    { c->setBounds(x, row3Y, kW, kH); x += kW + gap; }
}
