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

        KnobWithLabel rate, depth, phase;
        juce::ComboBox shapeBox;
        juce::Label    label, shapeLabel;

        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rateAtt, depthAtt, phaseAtt;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> shapeAtt;
    };

    std::array<std::unique_ptr<LFOStrip>, 4> lfoStrips;
    ModMatrixGrid modGrid;
};
