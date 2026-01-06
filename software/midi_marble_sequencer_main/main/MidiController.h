#pragma once

#include "MidiMapper.h"
#include "Sequencer.h"

#include <cstddef>

class MidiController
{
public:
    MidiController(Sequencer &sequencer);
    void send_eighth_note_midi_notes();
    void send_notes_off();
    void send_timing_clock();

private:
    Sequencer &_sequencer;
    MidiMapper _midi_mapper;

    midi_note_t _notes_on[SEQUENCER_EIGHTH_NOTE_NUM * SEQUENCER_TRACKS_NUM];
    size_t _notes_on_size;

    void _send_notes_on(midi_note_t *midi_notes, size_t num_notes);
};