#include "MidiMapperPlugin.h"
#include "Editor.h"


MidiMapperPluginProcessor::MidiMapperPluginProcessor()
    : AudioProcessor (getBusesLayout())
{
    state.addChild ({ "uiState", { { "width",  600 }, { "height", 300 } }, {} }, -1, nullptr);
}

void MidiMapperPluginProcessor::getStateInformation (MemoryBlock& destData)
{
    if (auto xmlState = state.createXml())
        copyXmlToBinary (*xmlState, destData);
}

void MidiMapperPluginProcessor::setStateInformation (const void* data, int size)
{
    if (auto xmlState = getXmlFromBinary (data, size))
        state = ValueTree::fromXml (*xmlState);
}

void MidiMapperPluginProcessor::loadMappingFromXml (const XmlElement* xmlRoot)
{
    midiMapping.loadFromXml (xmlRoot);
}

void MidiMapperPluginProcessor::clearMapping()
{
    midiMapping.clear();
}

AudioProcessor::BusesProperties MidiMapperPluginProcessor::getBusesLayout()
{
    const PluginHostType host;
    return host.isAbletonLive() || host.isSonar()
         ? BusesProperties().withOutput ("out", AudioChannelSet::stereo())
         : BusesProperties();
}
