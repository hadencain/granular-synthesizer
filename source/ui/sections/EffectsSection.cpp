#include "EffectsSection.h"

EffectsSection::EffectsSection(juce::AudioProcessorValueTreeState& apvts)
{
    auto att = [&](KnobWithLabel& k, const juce::String& id) {
        atts.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, id, k.getSlider()));
    };

    drive.setLabel("Drive");         att(drive, "fx_drive");
    wsLabel.setText("WS Type", juce::dontSendNotification);
    wsTypeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "fx_ws_type", wsTypeBox);

    const juce::String eqNames[][3] = {
        { "fx_eq1_freq","fx_eq1_gain","fx_eq1_q" },
        { "fx_eq2_freq","fx_eq2_gain","fx_eq2_q" },
        { "fx_eq3_freq","fx_eq3_gain","fx_eq3_q" },
        { "fx_eq4_freq","fx_eq4_gain","fx_eq4_q" }
    };
    for (int i = 0; i < 4; ++i)
    {
        eq[i].freq.setLabel("EQ" + juce::String(i+1) + " F");
        eq[i].gain.setLabel("Gain");
        eq[i].q.setLabel("Q");
        att(eq[i].freq, eqNames[i][0]);
        att(eq[i].gain, eqNames[i][1]);
        att(eq[i].q,    eqNames[i][2]);
    }

    chorusRate.setLabel("Rate");     att(chorusRate,    "fx_chorus_rate");
    chorusDepth.setLabel("Depth");   att(chorusDepth,   "fx_chorus_depth");
    chorusMix.setLabel("Mix");       att(chorusMix,     "fx_chorus_mix");
    delayTime.setLabel("Time");      att(delayTime,     "fx_delay_time_ms");
    delayFeedback.setLabel("Fdbk");  att(delayFeedback, "fx_delay_feedback");
    delayMix.setLabel("Mix");        att(delayMix,      "fx_delay_mix");
    reverbRoom.setLabel("Room");     att(reverbRoom,    "fx_reverb_room");
    reverbDamp.setLabel("Damp");     att(reverbDamp,    "fx_reverb_damp");
    reverbMix.setLabel("Mix");       att(reverbMix,     "fx_reverb_mix");
    limiterThresh.setLabel("Thresh");att(limiterThresh, "fx_limiter_thresh");
    limiterRelease.setLabel("Rel");  att(limiterRelease,"fx_limiter_release");
    pingPongAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "fx_delay_pingpong", pingPongBtn);

    addAndMakeVisible(drive);
    addAndMakeVisible(wsTypeBox); addAndMakeVisible(wsLabel);
    for (auto& band : eq) { addAndMakeVisible(band.freq); addAndMakeVisible(band.gain); addAndMakeVisible(band.q); }
    for (auto* c : { &chorusRate, &chorusDepth, &chorusMix,
                     &delayTime, &delayFeedback, &delayMix,
                     &reverbRoom, &reverbDamp, &reverbMix,
                     &limiterThresh, &limiterRelease })
        addAndMakeVisible(c);
    addAndMakeVisible(pingPongBtn);
}

void EffectsSection::resized()
{
    const int kW = 60, kH = 74, gap = 6;
    const int row1Y = 4, row2Y = row1Y + kH + gap, row3Y = row2Y + kH + gap, row4Y = row3Y + kH + gap;

    // Row 1: Drive + wsType + EQ bands
    int x = gap;
    drive.setBounds(x, row1Y, kW, kH); x += kW + gap;
    wsLabel.setBounds(x, row1Y, 70, 14);
    wsTypeBox.setBounds(x, row1Y + 16, 80, 20); x += 90;
    for (auto& band : eq)
    {
        band.freq.setBounds(x, row1Y, kW, kH); x += kW + 2;
        band.gain.setBounds(x, row1Y, kW, kH); x += kW + 2;
        band.q.setBounds   (x, row1Y, kW, kH); x += kW + gap;
    }

    // Row 2: Chorus
    x = gap;
    for (auto* c : { &chorusRate, &chorusDepth, &chorusMix })
    { c->setBounds(x, row2Y, kW, kH); x += kW + gap; }

    // Row 3: Delay
    x = gap;
    for (auto* c : { &delayTime, &delayFeedback, &delayMix })
    { c->setBounds(x, row3Y, kW, kH); x += kW + gap; }
    pingPongBtn.setBounds(x, row3Y + kH / 2 - 12, 80, 24);

    // Row 4: Reverb + Limiter
    x = gap;
    for (auto* c : { &reverbRoom, &reverbDamp, &reverbMix })
    { c->setBounds(x, row4Y, kW, kH); x += kW + gap; }
    x += gap * 2;
    for (auto* c : { &limiterThresh, &limiterRelease })
    { c->setBounds(x, row4Y, kW, kH); x += kW + gap; }
}
