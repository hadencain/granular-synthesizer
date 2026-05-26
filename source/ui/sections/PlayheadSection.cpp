#include "PlayheadSection.h"

PlayheadSection::PlayheadSection(juce::AudioProcessorValueTreeState& apvts)
    : posAtt      (apvts, "position",          position.getSlider()),
      sprayAtt    (apvts, "spray_ms",          spray.getSlider()),
      scanRateAtt (apvts, "scan_rate_hz",      scanRate.getSlider()),
      loopStartAtt(apvts, "loop_start",        loopStart.getSlider()),
      loopEndAtt  (apvts, "loop_end",          loopEnd.getSlider()),
      scrubAtt    (apvts, "scrub",             scrub.getSlider()),
      probAtt     (apvts, "grain_probability", prob.getSlider()),
      freezeAtt   (apvts, "freeze",            freezeBtn)
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
    prob.setLabel("Prob");
    scanShapeLabel.setText("Scan Shape", juce::dontSendNotification);

    for (auto* c : { &position, &spray, &scanRate, &loopStart, &loopEnd, &scrub })
        addAndMakeVisible(c);
    addAndMakeVisible(scanShapeBox);
    addAndMakeVisible(scanShapeLabel);
    addAndMakeVisible(freezeBtn);
    addAndMakeVisible(prob);
}

void PlayheadSection::resized()
{
    const int kW = 70, kH = 82, gap = 8, startX = 8, startY = 8;
    const int row2Y = startY + kH + gap;
    const int row3Y = row2Y + kH + gap;

    int x = startX;
    for (auto* c : { &position, &spray, &scanRate })
    { c->setBounds(x, startY, kW, kH); x += kW + gap; }

    x = startX;
    scanShapeLabel.setBounds(x, row2Y, 120, 14);
    scanShapeBox.setBounds(x, row2Y + 16, 120, 22);
    freezeBtn.setBounds(x + 130, row2Y + 16, 70, 28);
    prob.setBounds(214, row2Y, 60, kH);

    x = startX;
    for (auto* c : { &loopStart, &loopEnd, &scrub })
    { c->setBounds(x, row3Y, kW, kH); x += kW + gap; }
}
