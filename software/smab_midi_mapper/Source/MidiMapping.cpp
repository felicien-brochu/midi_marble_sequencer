#include "MidiMapping.h"

bool ActiveMappingKey::operator< (const ActiveMappingKey& other) const
{
    if (channel != other.channel) return channel < other.channel;
    return note < other.note;
}

bool ActiveMappingKey::operator== (const ActiveMappingKey& other) const
{
    return channel == other.channel && note == other.note;
}

MidiMapping::MidiMapping()
{
    clear();
}

void MidiMapping::clear()
{
    const ScopedLock sl (lock);
    for (auto& ch : mapping)
        for (auto& nm : ch)
            nm = NoteMapping{};
}

void MidiMapping::loadFromXml (const XmlElement* xmlRoot)
{
    if (xmlRoot == nullptr)
        return;

    if (! xmlRoot->hasTagName ("mapping"))
        return;

    clear();

    {
        const ScopedLock sl (lock);

        for (auto* channelIn = xmlRoot->getFirstChildElement(); channelIn != nullptr; channelIn = channelIn->getNextElement())
        {
            if (! channelIn->hasTagName ("channel-in"))
                continue;

            const int inChannelAttr = channelIn->getIntAttribute ("channel", -1);
            if (inChannelAttr < 0 || inChannelAttr >= 16)
                continue;

            const int inChanIdx = inChannelAttr;

            for (auto* noteIn = channelIn->getFirstChildElement(); noteIn != nullptr; noteIn = noteIn->getNextElement())
            {
                if (! noteIn->hasTagName ("note-in"))
                    continue;

                const int inNote = noteIn->getIntAttribute ("note", -1);
                if (inNote < 0 || inNote >= 128)
                    continue;

                for (auto* noteOut = noteIn->getFirstChildElement(); noteOut != nullptr; noteOut = noteOut->getNextElement())
                {
                    if (! noteOut->hasTagName ("note-out"))
                        continue;

                    const int outChannelAttr = noteOut->getIntAttribute ("channel", -1);
                    const int outNote = noteOut->getIntAttribute ("note", -1);
                    if (outChannelAttr < 0 || outChannelAttr >= 16 || outNote < 0 || outNote >= 128)
                        continue;

                    auto& mapEntry = mapping[inChanIdx][inNote];
                    if (mapEntry.numOuts < maxOutputsPerInput)
                    {
                        mapEntry.outChannel[mapEntry.numOuts] = outChannelAttr;
                        mapEntry.outNote[mapEntry.numOuts] = outNote;
                        ++mapEntry.numOuts;
                        mapEntry.mapped = (mapEntry.numOuts > 0);
                    }
                }
            }
        }
    }
}

void MidiMapping::saveToXml (XmlElement& mappingXml) const
{
    const ScopedLock sl (lock);
    
    for (int inChan = 0; inChan < 16; ++inChan)
    {
        XmlElement* channelElem = nullptr;

        for (int note = 0; note < 128; ++note)
        {
            const auto& m = mapping[inChan][note];
            if (! m.mapped || m.numOuts == 0) 
                continue;

            if (channelElem == nullptr)
            {
                channelElem = mappingXml.createNewChildElement ("channel-in");
                channelElem->setAttribute ("channel", inChan);
            }

            XmlElement* noteIn = channelElem->createNewChildElement ("note-in");
            noteIn->setAttribute ("note", note);

            for (int o = 0; o < m.numOuts; ++o)
            {
                XmlElement* noteOut = noteIn->createNewChildElement ("note-out");
                noteOut->setAttribute ("channel", m.outChannel[o]);
                noteOut->setAttribute ("note", m.outNote[o]);
            }
        }
    }
}

void MidiMapping::createDefaultMapping()
{
    const auto notes = getDefaultInputNotes();
    
    const ScopedLock sl (lock);
    
    for (int ch = 0; ch < 16; ++ch)
    {
        for (int n = 0; n < 128; ++n)
            mapping[ch][n] = NoteMapping{};
    }

    for (int inChan = 0; inChan < 8; ++inChan)
    {
        for (int note : notes)
        {
            auto& m = mapping[inChan][note];
            m.mapped = true;
            m.numOuts = 1;
            m.outChannel[0] = inChan;
            m.outNote[0] = note;
        }
    }
}

const std::array<std::array<NoteMapping, 128>, 16>& MidiMapping::getMapping() const
{
    return mapping;
}

std::array<std::array<NoteMapping, 128>, 16>& MidiMapping::getMapping()
{
    return mapping;
}

CriticalSection& MidiMapping::getLock()
{
    return lock;
}

bool MidiMapping::isMappingActive (int channel, int note) const
{
    const ScopedLock sl (activeLock);
    return activeMappings.find ({channel, note}) != activeMappings.end();
}

template <typename Element>
void MidiMapping::processMidiBuffer (AudioBuffer<Element>& audio, MidiBuffer& midiMessages)
{
    audio.clear();

    juce::MidiBuffer processedMidi;
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        const auto time = metadata.samplePosition;

        if (message.isNoteOn() || message.isNoteOff())
        {
            const int inChannel = message.getChannel() - 1;
            const int noteNumber = message.getNoteNumber();

            bool handled = false;

            std::array<int, maxOutputsPerInput> tmpOutChannels {};
            std::array<int, maxOutputsPerInput> tmpOutNotes {};
            int tmpNumOuts = 0;

            {
                const ScopedLock sl (lock);

                if (inChannel >= 0 && inChannel < 16 && noteNumber >= 0 && noteNumber < 128)
                {
                    const auto& m = mapping[inChannel][noteNumber];
                    if (m.mapped && m.numOuts > 0)
                    {
                        tmpNumOuts = m.numOuts;
                        for (int i = 0; i < tmpNumOuts; ++i)
                        {
                            tmpOutChannels[i] = m.outChannel[i];
                            tmpOutNotes[i]    = m.outNote[i];
                        }
                    }
                    else if (inChannel >= 8)
                    {
                        const int baseIn = inChannel - 8;
                        const auto& mb = mapping[baseIn][noteNumber];
                        if (mb.mapped && mb.numOuts > 0)
                        {
                            tmpNumOuts = mb.numOuts;
                            for (int i = 0; i < tmpNumOuts; ++i)
                            {
                                tmpOutChannels[i] = (mb.outChannel[i] + 8) % 16;
                                tmpOutNotes[i]    = mb.outNote[i];
                            }
                        }
                    }
                }
            }

            if (tmpNumOuts > 0)
            {
                for (int i = 0; i < tmpNumOuts; ++i)
                {
                    const int outChanJuce = tmpOutChannels[i] + 1;
                    const int outNote = tmpOutNotes[i];

                    if (message.isNoteOn())
                    {
                        processedMidi.addEvent (MidiMessage::noteOn (outChanJuce, outNote, (juce::uint8) message.getVelocity()), time);
                        
                        const ScopedLock sl (activeLock);
                        activeMappings.insert ({inChannel, noteNumber});
                    }
                    else
                    {
                        processedMidi.addEvent (MidiMessage::noteOff (outChanJuce, outNote, (juce::uint8) message.getVelocity()), time);
                        
                        const ScopedLock sl (activeLock);
                        activeMappings.erase ({inChannel, noteNumber});
                    }
                }

                handled = true;
            }

            if (! handled)
            {
                processedMidi.addEvent (message, time);
            }
        }
        else
        {
            processedMidi.addEvent (message, time);
        }
    }
    midiMessages.swapWith (processedMidi);
}

// Explicit template instantiations for the types used by AudioProcessor
template void MidiMapping::processMidiBuffer<float> (AudioBuffer<float>&, MidiBuffer&);
template void MidiMapping::processMidiBuffer<double> (AudioBuffer<double>&, MidiBuffer&);
