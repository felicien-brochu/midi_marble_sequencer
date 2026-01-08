#pragma once

#include "sequencer_config.h"
#include "marble_type.h"

#include <cstdint>


typedef enum {
    MEASURE_STATE_UNKNOWN = 0,
    MEASURE_STATE_SKIP,
    MEASURE_STATE_PLAY,
    MEASURE_STATE_LOCK,
    MEASURE_STATE_MAX
} measure_state_t;


class SequencerPage
{
public:
    SequencerPage();

    bool is_empty();
    bool has_playable_eighth_notes_after(uint8_t eighth_note_index);
    bool has_playable_eighth_notes();
    uint8_t get_first_playable_eighth_note_index();
    uint8_t get_first_playable_eighth_note_index_after(uint8_t eighth_note_index);
    void set_measures_states(const measure_state_t *measures_states);
    measure_state_t *get_measures_states();
    void set_eighth_note_marble_types(uint8_t eighth_note_index, marble_type_t *marble_types);
    void get_eighth_note_marble_types(uint8_t eighth_note_index, marble_type_t *marble_types);
    measure_state_t get_eighth_note_measure_state(uint8_t eighth_note_index);
    void set_measure_marble_types(uint8_t measure_index, marble_type_t *measure_marble_types);
    
    // Tick-based methods: convert tick count to eighth note index accounting for skipped measures
    uint8_t tick_to_eighth_note_index(uint8_t tick_count);
    bool has_more_ticks_after(uint8_t tick_count);
    uint8_t get_max_tick_count();
    void set_eighth_note_marble_types_from_tick(uint8_t tick_count, marble_type_t *marble_types);
    void get_eighth_note_marble_types_from_tick(uint8_t tick_count, marble_type_t *marble_types);
    measure_state_t get_eighth_note_measure_state_from_tick(uint8_t tick_count);

private:
    marble_type_t _marble_types[SEQUENCER_TRACKS_NUM * SEQUENCER_EIGHTH_NOTE_NUM];
    measure_state_t _measures_states[SEQUENCER_MEASURES_NUM];
};