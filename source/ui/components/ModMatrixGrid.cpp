#include "ModMatrixGrid.h"

ModMatrixGrid::ModMatrixGrid(ModMatrix& m) : modMatrix(m)
{
    addAndMakeVisible(table);

    table.setModel(this);
    table.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff1a1a2e));
    table.setRowHeight(24);
    table.setMultipleSelectionEnabled(false);

    auto& header = table.getHeader();
    header.addColumn("On",     COL_ENABLE, 30,  30,  30);
    header.addColumn("Source", COL_SOURCE, 80,  60,  120);
    header.addColumn("Target", COL_TARGET, 100, 80,  150);
    header.addColumn("Depth",  COL_DEPTH,  160, 100, 250);
    header.setStretchToFitActive(true);
}

void ModMatrixGrid::resized()
{
    table.setBounds(getLocalBounds());
}

int ModMatrixGrid::getNumRows()
{
    return ModMatrix::MAX_SLOTS;
}

void ModMatrixGrid::paintRowBackground(juce::Graphics& g, int row, int w, int h, bool selected)
{
    if (selected)
        g.fillAll(juce::Colour(0xff00d4ff).withAlpha(0.15f));
    else if (row % 2 == 0)
        g.fillAll(juce::Colour(0xff16213e));
    else
        g.fillAll(juce::Colour(0xff1a1a2e));
}

void ModMatrixGrid::paintCell(juce::Graphics& g, int row, int col, int w, int h, bool)
{
    // COL_SOURCE and COL_TARGET are rendered as components, not painted
    if (col == COL_ENABLE || col == COL_SOURCE || col == COL_TARGET || col == COL_DEPTH)
        return;

    g.setColour(juce::Colour(0xff909090));
    g.setFont(11.0f);
    juce::ignoreUnused(row, w, h);
}

juce::Component* ModMatrixGrid::refreshComponentForCell(int row, int col, bool,
                                                         juce::Component* existing)
{
    if (col == COL_ENABLE)
    {
        auto* btn = dynamic_cast<juce::ToggleButton*>(existing);
        if (btn == nullptr)
        {
            btn = new juce::ToggleButton();
            btn->setButtonText({});
        }
        btn->setToggleState(modMatrix.getSlot(row).active, juce::dontSendNotification);
        btn->onClick = [this, row, btn] {
            ModSlot s = modMatrix.getSlot(row);
            s.active = btn->getToggleState();
            modMatrix.setSlot(row, s.source, s.target, s.depth, s.active);
        };
        return btn;
    }

    if (col == COL_SOURCE)
    {
        auto* box = dynamic_cast<juce::ComboBox*>(existing);
        if (box == nullptr)
        {
            box = new juce::ComboBox();
            box->addItemList(sourceNames, 1);
            box->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff161616));
            box->setColour(juce::ComboBox::textColourId,       juce::Colour(0xffd0d0d0));
            box->setColour(juce::ComboBox::outlineColourId,    juce::Colour(0xff282828));
        }
        box->setSelectedItemIndex(static_cast<int>(modMatrix.getSlot(row).source),
                                  juce::dontSendNotification);
        box->onChange = [this, row, box] {
            ModSlot s = modMatrix.getSlot(row);
            s.source = static_cast<ModSource>(box->getSelectedItemIndex());
            modMatrix.setSlot(row, s.source, s.target, s.depth, s.active);
        };
        return box;
    }

    if (col == COL_TARGET)
    {
        auto* box = dynamic_cast<juce::ComboBox*>(existing);
        if (box == nullptr)
        {
            box = new juce::ComboBox();
            box->addItemList(targetNames, 1);
            box->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff161616));
            box->setColour(juce::ComboBox::textColourId,       juce::Colour(0xffd0d0d0));
            box->setColour(juce::ComboBox::outlineColourId,    juce::Colour(0xff282828));
        }
        box->setSelectedItemIndex(static_cast<int>(modMatrix.getSlot(row).target),
                                  juce::dontSendNotification);
        box->onChange = [this, row, box] {
            ModSlot s = modMatrix.getSlot(row);
            s.target = static_cast<ModTarget>(box->getSelectedItemIndex());
            modMatrix.setSlot(row, s.source, s.target, s.depth, s.active);
        };
        return box;
    }

    if (col == COL_DEPTH)
    {
        auto* slider = dynamic_cast<juce::Slider*>(existing);
        if (slider == nullptr)
        {
            slider = new juce::Slider(juce::Slider::LinearHorizontal,
                                      juce::Slider::NoTextBox);
            slider->setRange(-1.0, 1.0, 0.01);
        }
        slider->setValue(modMatrix.getSlot(row).depth, juce::dontSendNotification);
        slider->onValueChange = [this, row, slider] {
            ModSlot s = modMatrix.getSlot(row);
            s.depth = static_cast<float>(slider->getValue());
            modMatrix.setSlot(row, s.source, s.target, s.depth, s.active);
        };
        return slider;
    }

    delete existing;
    return nullptr;
}
