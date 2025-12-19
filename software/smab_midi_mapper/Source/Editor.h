#pragma once

#include <JuceHeader.h>
#include "PresetDisplay.h"

class MidiMapperPluginProcessor;

class Editor final : public AudioProcessorEditor,
                     private Value::Listener
{
public:
    explicit Editor (MidiMapperPluginProcessor& ownerIn);
    ~Editor() override = default;

    void paint (Graphics& g) override;
    void resized() override;

private:
    void onSelectedPresetChanged();
    void valueChanged (Value& value) override;
    void populatePresets();

    MidiMapperPluginProcessor& owner;

    ComboBox presetsBox;
    TextButton newButton { "New" };
    TextButton saveButton { "Save" };
    Value selectedPreset;
    PresetDisplay presetDisplay;

    Value lastUIWidth, lastUIHeight;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Editor)
};
