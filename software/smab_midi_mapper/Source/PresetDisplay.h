#pragma once

#include <JuceHeader.h>

class MidiMapperPluginProcessor;

class PresetDisplay : public juce::Component,
                      private juce::MidiKeyboardStateListener,
                      private juce::Timer
{
public:
    explicit PresetDisplay (MidiMapperPluginProcessor& ownerIn);
    ~PresetDisplay() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    // Convert a point to (row,col) cell indices. Returns true if inside grid.
    bool pointToCell (juce::Point<int> p, int& outRow, int& outCol) const;

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDoubleClick (const juce::MouseEvent& e) override;

    std::function<void(int row, int col)> onCellSelected;
    std::function<void(int row, int col)> onCellDoubleClicked;

private:
    MidiMapperPluginProcessor& owner;
    int selectedRow = -1;
    int selectedCol = -1;

    // Midi keyboard used to set output note for the selected cell. Disabled when no selection.
    juce::MidiKeyboardState keyboardState;
    std::unique_ptr<juce::MidiKeyboardComponent> keyboardComponent;
    int keyboardHeight = 80;

    // Row of channel buttons between grid and keyboard
    std::array<juce::TextButton, 8> channelButtons;
    int buttonRowHeight = 28;

    // MidiKeyboardStateListener
    void handleNoteOn (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float) override;
    void handleNoteOff (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float) override;
    
    // Timer callback for repainting to show active mappings
    void timerCallback() override;
};
