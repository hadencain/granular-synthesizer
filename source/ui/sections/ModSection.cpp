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
    depth.getSlider().onValueChange = [this] { repaint(); };
    addAndMakeVisible(phase);
}

void ModSection::LFOStrip::resized()
{
    const int w = getWidth();
    label.setBounds(0, 0, 40, 14);
    shapeLabel.setBounds(44, 0, 44, 14);
    shapeBox.setBounds(90, 0, w - 92, 22);
    const int kW = 70, kH = getHeight() - 28, gap = 6;
    int x = 0;
    rate.setBounds (x, 24, kW, kH); x += kW + gap;
    depth.setBounds(x, 24, kW, kH); x += kW + gap;
    phase.setBounds(x, 24, kW, kH);
}

void ModSection::LFOStrip::paint(juce::Graphics& g)
{
    const float depthVal = static_cast<float>(depth.getSlider().getValue());
    const bool  active   = depthVal > 0.001f;
    g.setColour(active ? juce::Colour(0xff8050b8) : juce::Colour(0xff909090).withAlpha(0.3f));
    g.fillEllipse(42.0f, 4.0f, 6.0f, 6.0f);
}

ModSection::EnvModStrip::EnvModStrip(juce::AudioProcessorValueTreeState& apvts, int n)
{
    auto ns = juce::String(n);
    label.setText("ENV " + ns, juce::dontSendNotification);
    attack.setLabel("Atk");
    decay.setLabel("Dec");
    sustain.setLabel("Sus");
    release.setLabel("Rel");

    atkAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "envmod" + ns + "_attack_ms",  attack.getSlider());
    decAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "envmod" + ns + "_decay_ms",   decay.getSlider());
    susAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "envmod" + ns + "_sustain",    sustain.getSlider());
    relAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "envmod" + ns + "_release_ms", release.getSlider());

    addAndMakeVisible(label);
    addAndMakeVisible(attack);
    addAndMakeVisible(decay);
    addAndMakeVisible(sustain);
    addAndMakeVisible(release);
}

void ModSection::EnvModStrip::resized()
{
    label.setBounds(0, 0, 60, 14);
    const int kW = 62, kH = getHeight() - 18, gap = 5;
    int x = 0;
    attack.setBounds (x, 18, kW, kH); x += kW + gap;
    decay.setBounds  (x, 18, kW, kH); x += kW + gap;
    sustain.setBounds(x, 18, kW, kH); x += kW + gap;
    release.setBounds(x, 18, kW, kH);
}

ModSection::ModSection(juce::AudioProcessorValueTreeState& apvts, ModMatrix& modMatrix)
    : modGrid(modMatrix)
{
    for (int i = 0; i < 4; ++i)
    {
        lfoStrips[i] = std::make_unique<LFOStrip>(apvts, i + 1);
        contentComp.addAndMakeVisible(*lfoStrips[i]);
    }

    for (int i = 0; i < 2; ++i)
    {
        envStrips[i] = std::make_unique<EnvModStrip>(apvts, i + 1);
        contentComp.addAndMakeVisible(*envStrips[i]);
    }

    efLabel.setText("FOLLOWER", juce::dontSendNotification);
    efAttack.setLabel("Atk");
    efRelease.setLabel("Rel");
    efAtkAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "ef_attack_ms",  efAttack.getSlider());
    efRelAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "ef_release_ms", efRelease.getSlider());
    contentComp.addAndMakeVisible(efLabel);
    contentComp.addAndMakeVisible(efAttack);
    contentComp.addAndMakeVisible(efRelease);

    contentComp.addAndMakeVisible(modGrid);

    routingLabel.setText("ROUTING", juce::dontSendNotification);
    routingLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    routingLabel.setColour(juce::Label::textColourId, juce::Colour(0xff808080));
    contentComp.addAndMakeVisible(routingLabel);

    sourcesLabel.setText("SOURCES", juce::dontSendNotification);
    sourcesLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    sourcesLabel.setColour(juce::Label::textColourId, juce::Colour(0xff808080));
    contentComp.addAndMakeVisible(sourcesLabel);

    viewport.setViewedComponent(&contentComp, false);
    viewport.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport);
}

void ModSection::resized()
{
    viewport.setBounds(getLocalBounds());

    const int w         = getWidth() - 16;
    const int stripH    = 110;
    const int envH      = 90;
    const int followerH = 90;
    const int matrixH   = 220;
    const int lblH      = 14;
    const int gap       = 6;

    const int totalH = 4 +
        lblH + 4 +
        matrixH + gap +
        lblH + 4 +
        4 * (stripH + gap) +
        2 * (envH   + gap) +
        followerH + gap;

    contentComp.setBounds(0, 0, getWidth(), totalH);

    int y = 4;

    // Routing grid at top — immediately visible
    routingLabel.setBounds(4, y, w, lblH);
    y += lblH + 4;
    modGrid.setBounds(4, y, w, matrixH);
    y += matrixH + gap;

    // LFO / ENV sources below
    sourcesLabel.setBounds(4, y, w, lblH);
    y += lblH + 4;

    for (auto& strip : lfoStrips)
    {
        strip->setBounds(4, y, w, stripH);
        y += stripH + gap;
    }

    for (auto& strip : envStrips)
    {
        strip->setBounds(4, y, w, envH);
        y += envH + gap;
    }

    efLabel.setBounds(4, y, 80, lblH);
    const int kW = 70, kH = followerH - 18;
    efAttack.setBounds (4,            y + 18, kW, kH);
    efRelease.setBounds(4 + kW + gap, y + 18, kW, kH);
}
