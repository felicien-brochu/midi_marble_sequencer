/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/
/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:                  smab_midi_mapper
 version:               1.0.0
 vendor:                Felicien Brochu
 website:               github.com/felicien-brochu
 description:           Maps SMAB midi messages using mapping presets.

 dependencies:          juce_audio_basics, juce_audio_devices, juce_audio_formats,
                        juce_audio_plugin_client, juce_audio_processors,
                        juce_audio_utils, juce_core, juce_data_structures,
                        juce_events, juce_graphics, juce_gui_basics, juce_gui_extra,
                        juce_audio_processors_headless
 exporters:             xcode_mac, vs2022, vs2026, linux_make

 moduleFlags:           JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:                  AudioProcessor
 mainClass:             MidiMapperPluginProcessor

 useLocalCopy:          1

 pluginCharacteristics: pluginWantsMidiIn, pluginProducesMidiOut, pluginIsMidiEffectPlugin

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include <iterator>
#include <array>
#include "paths.h"
#include "PresetDisplay.h"
#include "Editor.h"
#include "MidiMapping.h"

//==============================================================================
class MidiMapperPluginProcessor final : public AudioProcessor
{
public:

    friend class Editor;

    MidiMapperPluginProcessor();
    ~MidiMapperPluginProcessor() override = default;

    void processBlock (AudioBuffer<float>& audio,  MidiBuffer& midi) override { midiMapping.processMidiBuffer (audio, midi); }
    void processBlock (AudioBuffer<double>& audio, MidiBuffer& midi) override { midiMapping.processMidiBuffer (audio, midi); }

    bool isBusesLayoutSupported (const BusesLayout&) const override           { return true; }
    bool isMidiEffect() const override                                        { return true; }
    bool hasEditor() const override                                           { return true; }
    AudioProcessorEditor* createEditor() override                             { return new Editor (*this); }

    const String getName() const override                                     { return "SMAB midi mapper"; }
    bool acceptsMidi() const override                                         { return true; }
    bool producesMidi() const override                                        { return true; }
    double getTailLengthSeconds() const override                              { return 0.0; }

    int getNumPrograms() override                                             { return 0; }
    int getCurrentProgram() override                                          { return 0; }
    void setCurrentProgram (int) override                                     {}
    const String getProgramName (int) override                                { return "None"; }
    void changeProgramName (int, const String&) override                      {}

    void prepareToPlay (double, int) override                                 {}
    void releaseResources() override                                          {}

    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int size) override;

    void loadMappingFromXml (const XmlElement* xmlRoot);
    void clearMapping();

    MidiMapping& getMidiMapping() { return midiMapping; }
    const MidiMapping& getMidiMapping() const { return midiMapping; }

private:
    static BusesProperties getBusesLayout();

    ValueTree state { "state" };
    MidiMapping midiMapping;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiMapperPluginProcessor)
};
