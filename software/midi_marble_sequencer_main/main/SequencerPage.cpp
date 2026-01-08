#include "SequencerPage.h"

#include <cstring>
#include <esp_log.h>

static const char *TAG = "SEQUENCER_PAGE";


SequencerPage::SequencerPage()
{
    for (size_t i = 0; i < SEQUENCER_TRACKS_NUM * SEQUENCER_EIGHTH_NOTE_NUM; i++)
    {
        _marble_types[i] = NO_MARBLE;
    }

    for (size_t i = 0; i < SEQUENCER_MEASURES_NUM; i++)
    {
        _measures_states[i] = MEASURE_STATE_PLAY;
    }
}

bool SequencerPage::is_empty()
{
    for (size_t i = 0; i < SEQUENCER_TRACKS_NUM * SEQUENCER_EIGHTH_NOTE_NUM; i++)
    {
        if (_marble_types[i] != NO_MARBLE)
        {
            return false;
        }
    }

    return true;
}

static inline uint8_t _eighth_note_index_to_measure_index(uint8_t eighth_note_index)
{
    return eighth_note_index / SEQUENCER_EIGHTH_NOTE_BY_MEASURE_NUM;
}

static inline uint8_t _eighth_note_index_in_measure(uint8_t eighth_note_index, uint8_t measure_index)
{
    return eighth_note_index - (measure_index * SEQUENCER_EIGHTH_NOTE_BY_MEASURE_NUM);
}

bool SequencerPage::has_playable_eighth_notes_after(uint8_t eighth_note_index)
{
    if (!has_playable_eighth_notes())
    {
        return false;
    }

    uint8_t measure_index = _eighth_note_index_to_measure_index(eighth_note_index);

    uint8_t last_playable_measure_index = 0;
    for (size_t i = 0; i < SEQUENCER_MEASURES_NUM; i++)
    {
        if (_measures_states[i] != MEASURE_STATE_SKIP)
        {
            last_playable_measure_index = i;
        }
    }

    if (last_playable_measure_index < measure_index)
    {
        return false;
    }
    
    if (last_playable_measure_index == measure_index && _eighth_note_index_in_measure(eighth_note_index, measure_index) >= SEQUENCER_EIGHTH_NOTE_BY_MEASURE_NUM - 1)
    {
        return false;
    }

    return true;
}

bool SequencerPage::has_playable_eighth_notes()
{
    for (size_t i = 0; i < SEQUENCER_MEASURES_NUM; i++)
    {
        if (_measures_states[i] != MEASURE_STATE_SKIP)
        {
            return true;
        }
    }

    return false;
}

uint8_t SequencerPage::get_first_playable_eighth_note_index()
{
    uint8_t measure_index;
    for (measure_index = 0; measure_index < SEQUENCER_MEASURES_NUM; measure_index++)
    {
        if (_measures_states[measure_index] != MEASURE_STATE_SKIP)
        {
            break;
        }
    }

    if (measure_index >= SEQUENCER_MEASURES_NUM)
    {
        return SEQUENCER_EIGHTH_NOTE_NUM;
    }

    return measure_index * SEQUENCER_EIGHTH_NOTE_BY_MEASURE_NUM;
}


uint8_t SequencerPage::get_first_playable_eighth_note_index_after(uint8_t eighth_note_index)
{
    if (!has_playable_eighth_notes())
    {
        return SEQUENCER_EIGHTH_NOTE_NUM;
    }

    uint8_t measure_index = _eighth_note_index_to_measure_index(eighth_note_index);

    if (_measures_states[measure_index] != MEASURE_STATE_SKIP && _eighth_note_index_in_measure(eighth_note_index, measure_index) < SEQUENCER_EIGHTH_NOTE_BY_MEASURE_NUM - 1)
    {
        return eighth_note_index + 1;
    }

    for (size_t i = measure_index + 1; i < SEQUENCER_MEASURES_NUM; i++)
    {
        if (_measures_states[i] != MEASURE_STATE_SKIP)
        {
            return i * SEQUENCER_EIGHTH_NOTE_BY_MEASURE_NUM;
        }
    }

    return SEQUENCER_EIGHTH_NOTE_NUM;
}

void SequencerPage::set_measures_states(const measure_state_t *measures_states)
{
    memcpy(_measures_states, measures_states, SEQUENCER_MEASURES_NUM * sizeof(measure_state_t));
}

measure_state_t *SequencerPage::get_measures_states()
{
    return _measures_states;
}

void SequencerPage::set_eighth_note_marble_types(uint8_t eighth_note_index, marble_type_t *marble_types)
{
    memcpy(&_marble_types[eighth_note_index * SEQUENCER_TRACKS_NUM], marble_types, SEQUENCER_TRACKS_NUM * sizeof(marble_type_t));
}

void SequencerPage::get_eighth_note_marble_types(uint8_t eighth_note_index, marble_type_t *marble_types)
{
    if (eighth_note_index >= SEQUENCER_EIGHTH_NOTE_NUM)
    {
        memset(marble_types, NO_MARBLE, SEQUENCER_TRACKS_NUM * sizeof(marble_type_t));
        return;
    }
    else
    {
        memcpy(marble_types, &_marble_types[eighth_note_index * SEQUENCER_TRACKS_NUM], SEQUENCER_TRACKS_NUM * sizeof(marble_type_t));
    }
}

measure_state_t SequencerPage::get_eighth_note_measure_state(uint8_t eighth_note_index)
{
    return _measures_states[_eighth_note_index_to_measure_index(eighth_note_index)];
}

void SequencerPage::set_measure_marble_types(uint8_t measure_index, marble_type_t *measure_marble_types)
{
    const uint8_t marbles_by_measure = SEQUENCER_EIGHTH_NOTE_BY_MEASURE_NUM * SEQUENCER_TRACKS_NUM;
    memcpy(&_marble_types[measure_index * marbles_by_measure], measure_marble_types, marbles_by_measure * sizeof(marble_type_t));
}

// Tick-based methods implementation
// 
// These methods allow working with a "tick count" which represents the number of
// eighth notes played from the start of the page, treating skipped measures as if
// they don't exist. The tick_to_eighth_note_index() method converts this linear
// tick count to the actual eighth_note_index in the full 32-position array,
// accounting for which measures are set to SKIP.
//
// Example: If measures 0 and 2 are PLAY, and measures 1 and 3 are SKIP:
// - tick 0 -> eighth_note_index 0
// - tick 7 -> eighth_note_index 7
// - tick 8 -> eighth_note_index 16 (start of measure 2)
// - tick 15 -> eighth_note_index 23 (last note of measure 2)
// - max_tick_count = 16 (only 2 measures * 8 notes each)

uint8_t SequencerPage::tick_to_eighth_note_index(uint8_t tick_count)
{
    if (!has_playable_eighth_notes())
    {
        return SEQUENCER_EIGHTH_NOTE_NUM;
    }
    
    uint8_t ticks_counted = 0;
    
    for (uint8_t measure_index = 0; measure_index < SEQUENCER_MEASURES_NUM; measure_index++)
    {
        if (_measures_states[measure_index] != MEASURE_STATE_SKIP)
        {
            // This measure is playable
            uint8_t ticks_in_measure = SEQUENCER_EIGHTH_NOTE_BY_MEASURE_NUM;
            
            if (ticks_counted + ticks_in_measure > tick_count)
            {
                // The tick is within this measure
                uint8_t tick_in_measure = tick_count - ticks_counted;
                return measure_index * SEQUENCER_EIGHTH_NOTE_BY_MEASURE_NUM + tick_in_measure;
            }
            
            ticks_counted += ticks_in_measure;
        }
    }
    
    // Tick count exceeds available ticks
    return SEQUENCER_EIGHTH_NOTE_NUM;
}

bool SequencerPage::has_more_ticks_after(uint8_t tick_count)
{
    // Check if there are more ticks after the given tick_count
    // tick_count + 1 would be the next tick
    return (tick_count + 1) < get_max_tick_count();
}

uint8_t SequencerPage::get_max_tick_count()
{
    uint8_t total_ticks = 0;
    
    for (uint8_t measure_index = 0; measure_index < SEQUENCER_MEASURES_NUM; measure_index++)
    {
        if (_measures_states[measure_index] != MEASURE_STATE_SKIP)
        {
            total_ticks += SEQUENCER_EIGHTH_NOTE_BY_MEASURE_NUM;
        }
    }
    
    return total_ticks;
}

void SequencerPage::set_eighth_note_marble_types_from_tick(uint8_t tick_count, marble_type_t *marble_types)
{
    uint8_t eighth_note_index = tick_to_eighth_note_index(tick_count);
    set_eighth_note_marble_types(eighth_note_index, marble_types);
}

void SequencerPage::get_eighth_note_marble_types_from_tick(uint8_t tick_count, marble_type_t *marble_types)
{
    uint8_t eighth_note_index = tick_to_eighth_note_index(tick_count);
    get_eighth_note_marble_types(eighth_note_index, marble_types);
}

measure_state_t SequencerPage::get_eighth_note_measure_state_from_tick(uint8_t tick_count)
{
    uint8_t eighth_note_index = tick_to_eighth_note_index(tick_count);
    return get_eighth_note_measure_state(eighth_note_index);
}