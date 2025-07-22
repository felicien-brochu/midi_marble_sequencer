#include "marble_type.h"
#include <cstdint>
#include <cstddef>

class MidiMapper
{
public:
    MidiMapper();

    size_t eighth_note_marble_types_to_midi_notes(uint8_t *midi_notes, const marble_type_t *marble_types);

private:
    uint8_t _eighth_note_marble_type_to_midi_note(uint8_t line_index, marble_type_t marble_type);
};