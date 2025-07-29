#include "SequencerPage.h"

#include <cstring>
#include <esp_log.h>

static const char *TAG = "SEQUENCER_PAGE";


SequencerPage::SequencerPage()
{
    for (size_t i = 0; i < SEQUENCER_MEASURES_NUM; i++)
    {
        _measures_states[i] = MEASURE_STATE_PLAY;
    }
}

void SequencerPage::record_measure(marble_type_t *measure_marble_types, uint8_t measure_index)
{
    memcpy(_marble_types + measure_index * SEQUENCER_MARBLE_BY_MEASURE_NUM, measure_marble_types, SEQUENCER_MARBLE_BY_MEASURE_NUM * sizeof(marble_type_t));
}

void SequencerPage::record_page(marble_type_t *page_marble_types)
{
    memcpy(_marble_types, page_marble_types, SEQUENCER_TRACKS_NUM * SEQUENCER_EIGHTH_NOTE_NUM * sizeof(marble_type_t));
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