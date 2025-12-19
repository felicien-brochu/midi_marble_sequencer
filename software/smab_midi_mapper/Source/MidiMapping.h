#pragma once

#include <JuceHeader.h>
#include <array>
#include <set>

static constexpr int maxOutputsPerInput = 8;

struct NoteMapping
{
    bool mapped = false;
    int numOuts = 0;
    std::array<int, maxOutputsPerInput> outChannel{};
    std::array<int, maxOutputsPerInput> outNote{};
};

struct ActiveMappingKey
{
    int channel;
    int note;
    
    bool operator< (const ActiveMappingKey& other) const;
    bool operator== (const ActiveMappingKey& other) const;
};

class MidiMapping
{
public:
    static constexpr std::array<int, 5> getDefaultInputNotes() { return std::array<int, 5>{ 60, 63, 65, 67, 70 }; }

    MidiMapping();

    void clear();
    void loadFromXml (const XmlElement* xmlRoot);
    void saveToXml (XmlElement& mappingXml) const;
    void createDefaultMapping();
    
    template <typename Element>
    void processMidiBuffer (AudioBuffer<Element>& audio, MidiBuffer& midiMessages);

    const std::array<std::array<NoteMapping, 128>, 16>& getMapping() const;
    std::array<std::array<NoteMapping, 128>, 16>& getMapping();
    CriticalSection& getLock();
    
    bool isMappingActive (int channel, int note) const;

private:
    std::array<std::array<NoteMapping, 128>, 16> mapping;
    CriticalSection lock;
    
    mutable CriticalSection activeLock;
    std::set<ActiveMappingKey> activeMappings;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiMapping)
};
