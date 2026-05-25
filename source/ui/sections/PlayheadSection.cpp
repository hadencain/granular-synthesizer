#include "PlayheadSection.h"

PlayheadSection::PlayheadSection(juce::AudioProcessorValueTreeState& apvts)
    : posAtt      (apvts, "position",     position.getSlider()),
      sprayAtt    (apvts, "spray_ms",     spray.getSlider()),
      scanRateAtt (apvts, "scan_rate_hz", scanRate.getSlider()),
      loopStartAtt(apvts, "loop_start",   loopStart.getSlider()),
      loopEndAtt  (apvts, "loop_end",     loopEnd.getSlider()),
      scrubAtt    (apvts, "scrub",        scrub.getSlider()),
      freezeAtt   (apvts, "freeze",       freezeBtn)
{
    scanShapeBox.addItemList({"Forward","Reverse","Pendulum","Random Walk","Sine"}, 1);
    scanShapeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "scan_shape", scanShapeBox);
    position.setLabel("Position");
    spray.setLabel("Spray");
    scanRate.setLabel("Scan Rate");
    loopStart.setLabel("Loop Start");
    loopEnd.setLabel("Loop End");
    scrub.setLabel("Scrub");
    scanShapeLabel.setText("Scan Shape", juce::dontSendNotification);

    for (auto* c : { &position, &spray, &scanRate, &loopStart, &loopEnd, &scrub })
        addAndMakeVisible(c);
    addAndMakeVisible(scanShapeBox);
    addAndMakeVisible(scanShapeLabel);
    addAndMakeVisible(freezeBtn);
}

void PlayheadSection::resized()
{
    const int knobW = 80, knobH = 90, gap = 10;
    const int row1Y = 10, row2Y = row1Y + knobH + gap;
    int x = gap;

    // Position knob is extra large (120px)
    position.setBounds(x, row1Y, 120, knobH);
    x += 130;
    for (auto* c : { &spray, &scanRate })
    {
        c->setBounds(x, row1Y, knobW, knobH);
        x += knobW + gap;
    }
    scanShapeLabel.setBounds(x, row1Y, knobW + 20, 14);
    scanShapeBox.setBounds(x, row1Y + 16, knobW + 20, 22);

    x = gap;
    for (auto* c : { &loopStart, &loopEnd, &scrub })
    {
        c->setBounds(x, row2Y, knobW, knobH);
        x += knobW + gap;
    }
    freezeBtn.setBounds(x, row2Y + 30, 70, 28);
}
