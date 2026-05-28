#include "FilterSection.h"

FilterSection::FilterSection(juce::AudioProcessorValueTreeState& apvts)
    : cutoffAtt  (apvts, "filter_cutoff_hz",  cutoff.getSlider()),
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

    for (auto* c : { &cutoff, &resonance, &envDepth, &lfoDepth, &keytrack })
        addAndMakeVisible(c);
    addAndMakeVisible(filterTypeBox);
    addAndMakeVisible(filterTypeLabel);
}

void FilterSection::resized()
{
    const int kW = 70, kH = 82, gap = 8, startX = 8, startY = 8;
    const int row2Y = startY + kH + gap;

    filterTypeLabel.setBounds(startX, startY, 110, 14);
    filterTypeBox.setBounds(startX, startY + 16, 110, 22);

    int x = startX;
    for (auto* c : { &cutoff, &resonance, &envDepth })
    { c->setBounds(x, row2Y, kW, kH); x += kW + gap; }

    x = startX;
    const int row3Y = row2Y + kH + gap;
    for (auto* c : { &lfoDepth, &keytrack })
    { c->setBounds(x, row3Y, kW, kH); x += kW + gap; }
}
