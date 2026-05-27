#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../components/KnobWithLabel.h"
#include "../components/ModMatrixGrid.h"

class ModSection : public juce::Component
{
public:
    ModSection(juce::AudioProcessorValueTreeState& apvts, ModMatrix& modMatrix);
    void resized() override;

private:
    struct LFOStrip : public juce::Component
    {
        explicit LFOStrip(juce::AudioProcessorValueTreeState& apvts, int n);
        void resized() override;
        void paint(juce::Graphics& g) override;

        KnobWithLabel rate, depth, phase;
        juce::ComboBox shapeBox;
        juce::Label    label, shapeLabel;

        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rateAtt, depthAtt, phaseAtt;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> shapeAtt;
    };

    struct EnvModStrip : public juce::Component
    {
        explicit EnvModStrip(juce::AudioProcessorValueTreeState& apvts, int n);
        void resized() override;

        KnobWithLabel attack, decay, sustain, release;
        juce::Label   label;

        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
            atkAtt, decAtt, susAtt, relAtt;
    };

    std::array<std::unique_ptr<LFOStrip>, 4>    lfoStrips;
    std::array<std::unique_ptr<EnvModStrip>, 2>  envStrips;

    // Envelope Follower (inline — only 2 knobs)
    KnobWithLabel efAttack, efRelease;
    juce::Label   efLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> efAtkAtt, efRelAtt;

    ModMatrixGrid        modGrid;
    juce::Component      contentComp;
    juce::Viewport       viewport;

    juce::Label routingLabel, sourcesLabel;
};
