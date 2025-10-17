#include "MidiMapper.h"
#include "IRSensBoards.h"
#include "sequencer_config.h"

static const uint8_t marble_types_melodic_mapping[] = {
    36, 38, 39, 41, 43, 45, 46,
    48, 50, 51, 53, 55, 57, 58,
    60, 62, 63, 65, 67, 69, 70,
    72, 74, 75, 77, 79, 81, 82,
    84, 86, 87, 89, 91, 93, 94,
    96, 98, 99, 101, 103, 105, 106,
    108, 110, 111, 113, 115, 117, 118,
};

MidiMapper::MidiMapper()
{
}

size_t MidiMapper::eighth_note_marble_types_to_midi_notes(midi_note_t *midi_notes, const marble_type_t *marble_types, const bool *enabled_tracks)
{
    size_t num_midi_notes = 0;
    for (uint8_t track_index = 0; track_index < SEQUENCER_TRACKS_NUM; track_index++)
    {
        if (enabled_tracks[track_index])
        {
            marble_type_t marble_type = marble_types[track_index];
            if (marble_type != NO_MARBLE)
            {
                midi_notes[num_midi_notes] = _eighth_note_marble_type_to_midi_note_boom_box(track_index, marble_type);
                num_midi_notes++;
            }
        }
    }
    
    return num_midi_notes;
}

midi_note_t MidiMapper::_eighth_note_marble_type_to_midi_note_melodic(uint8_t track_index, marble_type_t marble_type)
{
    uint8_t track_index_inv = SEQUENCER_TRACKS_NUM - track_index - 1;
    uint8_t marble_type_index = (marble_type - 1);

    midi_note_t midi_note = {
        .note = marble_types_melodic_mapping[marble_type_index * SEQUENCER_TRACKS_NUM + track_index_inv],
        .channel = 0
    };

    return midi_note;
}

midi_note_t MidiMapper::_eighth_note_marble_type_to_midi_note_boom_box(uint8_t track_index, marble_type_t marble_type)
{
    uint8_t notes[] = {53, 60, 67};
    uint8_t marble_type_index = (marble_type - 1);

    midi_note_t midi_note = {
        .note = notes[marble_type_index],
        .channel = track_index
    };

    return midi_note;
}
