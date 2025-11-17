#include "marble_type.h"
#include <cstdint>
#include <cstddef>

typedef struct {
    uint8_t note;
    uint8_t channel;
} midi_note_t;

class MidiMapper
{
public:
    MidiMapper();

    size_t eighth_note_marble_types_to_midi_notes(midi_note_t *midi_notes, const marble_type_t *marble_types, const bool *enabled_tracks);

private:
    midi_note_t _eighth_note_marble_type_to_midi_note_live_1(uint8_t track_index, marble_type_t marble_type);
    midi_note_t _eighth_note_marble_type_to_midi_note_melodic(uint8_t line_index, marble_type_t marble_type);
    midi_note_t _eighth_note_marble_type_to_midi_note_boom_box(uint8_t line_index, marble_type_t marble_type);
};