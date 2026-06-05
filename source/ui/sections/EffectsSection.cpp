#include "EffectsSection.h"

EffectsSection::EffectsSection(juce::AudioProcessorValueTreeState& apvts)
    : reverbBlob(apvts),
      eqDisplay(apvts)
{
    auto att = [&](KnobWithLabel& k, const juce::String& id) {
        atts.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, id, k.getSlider()));
    };

    drive.setLabel("Drive");         att(drive, "fx_drive");
    wsLabel.setText("WS Type", juce::dontSendNotification);
    wsTypeBox.addItemList({"Tanh", "Soft Clip"}, 1);
    wsTypeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "fx_ws_type", wsTypeBox);

    for (int i = 0; i < 4; ++i)
    {
        eqQAtts[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "fx_eq" + juce::String(i + 1) + "_q", eqQSliders[i]);
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
    addAndMakeVisible(eqDisplay);
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

    // EQ curve display
    eqDisplay.setBounds(startX, y, getWidth() - startX * 2, 80);
    y += 80 + gap;

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

    // Reverb — 3 knobs row, then blob below spanning full width
    x = startX;
    for (auto* c : { &reverbRoom, &reverbDamp, &reverbMix })
    { c->setBounds(x, y, kW, kH); x += kW + gap; }
    y += kH + gap;
    reverbBlob.setBounds(startX, y, getWidth() - startX * 2, 48);
    y += 48 + gap;

    // Limiter
    x = startX;
    for (auto* c : { &limiterThresh, &limiterRelease })
    { c->setBounds(x, y, kW, kH); x += kW + gap; }
}
