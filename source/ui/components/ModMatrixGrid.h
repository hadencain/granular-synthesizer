#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../../modulation/ModMatrix.h"

class ModMatrixGrid : public juce::Component,
                      public juce::TableListBoxModel
{
public:
    explicit ModMatrixGrid(ModMatrix& matrix);

    // juce::TableListBoxModel
    int  getNumRows() override;
    void paintRowBackground(juce::Graphics&, int row, int w, int h, bool selected) override;
    void paintCell(juce::Graphics&, int row, int col, int w, int h, bool selected) override;
    juce::Component* refreshComponentForCell(int row, int col, bool selected,
                                              juce::Component* existing) override;

    void resized() override;

private:
    ModMatrix&        modMatrix;
    juce::TableListBox table { "ModMatrix", this };

    static constexpr int COL_ENABLE = 1;
    static constexpr int COL_SOURCE = 2;
    static constexpr int COL_TARGET = 3;
    static constexpr int COL_DEPTH  = 4;

    juce::StringArray sourceNames { "LFO1","LFO2","LFO3","LFO4","Env1","Env2",
                                    "Vel","AT","CC1","EF","Rand" };
    juce::StringArray targetNames { "Size","Density","Pitch","Amp","Pan",
                                    "Position","Spray","ScanRate",
                                    "FltCut","FltRes","PitchSt","Detune",
                                    "Rate","Amplitude","MasterVol" };
};
