#include "EffectsSection.h"

EffectsSection::EffectsSection(juce::AudioProcessorValueTreeState& apvts)
    : reverbBlob(apvts)
{
    auto att = [&](KnobWithLabel& k, const juce::String& id) {
        atts.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, id, k.getSlider()));
    };

    drive.setLabel("Drive");         att(drive, "fx_drive");
    wsLabel.setText("WS Type", juce::dontSendNotification);
    wsTypeBox.addItemList({"Tanh", "Soft Clip"}, 1);
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
    addAndMakeVisible(reverbBlob);
}

void EffectsSection::resized()
{
    const int kW = 60, kH = 54, gap = 4, startX = 6;
    int y = 4;

    // Row 0: Drive + WS type
    drive.setBounds(startX, y, kW, kH);
    wsLabel.setBounds(startX + kW + gap, y, 80, 14);
    wsTypeBox.setBounds(startX + kW + gap, y + 16, 100, 22);
    y += kH + gap;

    // EQ bands — 2 per row (each band: freq + gain + q)
    // Row 1: bands 0,1  Row 2: bands 2,3
    for (int row = 0; row < 2; ++row)
    {
        int x = startX;
        for (int b = row * 2; b < row * 2 + 2; ++b)
        {
            eq[b].freq.setBounds(x, y, kW, kH); x += kW + 2;
            eq[b].gain.setBounds(x, y, kW, kH); x += kW + 2;
            eq[b].q.setBounds   (x, y, kW, kH); x += kW + gap + 4;
        }
        y += kH + gap;
    }

    // Chorus
    int x = startX;
    for (auto* c : { &chorusRate, &chorusDepth, &chorusMix })
    { c->setBounds(x, y, kW, kH); x += kW + gap; }
    y += kH + gap;

    // Delay
    x = startX;
    for (auto* c : { &delayTime, &delayFeedback, &delayMix })
    { c->setBounds(x, y, kW, kH); x += kW + gap; }
    pingPongBtn.setBounds(x, y + kH / 2 - 12, 80, 24);
    y += kH + gap;

    // Reverb — 3 knobs + blob to the right
    x = startX;
    for (auto* c : { &reverbRoom, &reverbDamp, &reverbMix })
    { c->setBounds(x, y, kW, kH); x += kW + gap; }
    reverbBlob.setBounds(x, y + 4, 66, kH - 8);
    y += kH + gap;

    // Limiter
    x = startX;
    for (auto* c : { &limiterThresh, &limiterRelease })
    { c->setBounds(x, y, kW, kH); x += kW + gap; }
}
