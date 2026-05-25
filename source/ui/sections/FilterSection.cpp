#include "FilterSection.h"

FilterSection::FilterSection(juce::AudioProcessorValueTreeState& apvts)
    : cutoffAtt  (apvts, "filter_cutoff_hz",  cutoff.getSlider()),
      resAtt     (apvts, "filter_resonance",   resonance.getSlider()),
      envDepthAtt(apvts, "filter_env_depth",   envDepth.getSlider()),
      lfoDepthAtt(apvts, "filter_lfo_depth",   lfoDepth.getSlider()),
      keytrackAtt(apvts, "filter_keytrack",    keytrack.getSlider()),
      typeAtt    (apvts, "filter_type",        filterTypeBox)
{
    cutoff.setLabel("Cutoff");
    resonance.setLabel("Resonance");
    envDepth.setLabel("Env Depth");
    lfoDepth.setLabel("LFO Depth");
    keytrack.setLabel("Keytrack");
    filterTypeLabel.setText("Type", juce::dontSendNotification);

    for (auto* c : { &cutoff, &resonance, &envDepth, &lfoDepth, &keytrack })
        addAndMakeVisible(c);
    addAndMakeVisible(filterTypeBox);
    addAndMakeVisible(filterTypeLabel);
}

void FilterSection::resized()
{
    const int knobW = 80, knobH = 90, gap = 10;
    int x = gap;
    filterTypeLabel.setBounds(x, 10, 90, 14);
    filterTypeBox.setBounds  (x, 26, 90, 22);
    x += 100;
    for (auto* c : { &cutoff, &resonance, &envDepth, &lfoDepth, &keytrack })
    { c->setBounds(x, 10, knobW, knobH); x += knobW + gap; }
}
