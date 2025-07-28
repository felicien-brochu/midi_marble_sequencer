#include "Sequencer.h"

#include <cstring>
#include <cstddef>
#include <esp_log.h>

static const char *TAG = "SEQUENCER";

Sequencer::Sequencer()
{
    for (size_t i = 0; i < SEQUENCER_PAGES_NUM; i++)
    {
        _played_pages[i] = i == 0 ? true : false;
    }
    
    _edited_page_index = 0;

    _current_page_index = 0;
    _current_eighth_note_index = 0;

    _is_playing = false;
    _bpm = SEQUENCER_BPM_DEFAULT;

    for (size_t i = 0; i < SEQUENCER_TRACKS_NUM; i++)
    {
        _tracks_enabled[i] = true;
    }

    for (size_t i = 0; i < SEQUENCER_MEASURES_NUM; i++)
    {
        _measures_states[i] = MEASURE_STATE_PLAY;
    }
}

void Sequencer::set_sequencer_callback(sequencer_callback_t sequencer_callback, void *sequencer_callback_context)
{
    _sequencer_callback = sequencer_callback;
    _sequencer_callback_context = sequencer_callback_context;
}

controls_main_display_t Sequencer::get_controls_main_display()
{
    controls_main_display_t controls_main_display;
    controls_main_display.play_pause_led_enabled = _is_playing;
    memcpy(controls_main_display.tracks_led_enabled, _tracks_enabled, sizeof(_tracks_enabled));

    return controls_main_display;
}

void Sequencer::handle_controls_main_event(const controls_main_value_t controls_main_value)
{
    _set_playing_from_play_pause_switch(controls_main_value.play_pause_switch_pushed);
    _set_bpm_from_potentiometer(controls_main_value.bpm_potentiometer_value);
    _set_tracks_enabled_from_push_buttons(controls_main_value.tracks_push_buttons);
    _set_measures_states_from_rotary_buttons(controls_main_value.rotary_buttons_states);
}

bool Sequencer::is_playing()
{
    return _is_playing;
}

uint64_t Sequencer::get_eighth_note_duration()
{
    // printf("BPM: %f, 8thnote duration: %lld\n", _bpm, (uint64_t) (60000000. / _bpm));
    return (uint64_t) (60000000. / _bpm);
}

uint8_t Sequencer::get_current_eighth_note_index()
{
    return _current_eighth_note_index;
}

void Sequencer::next_eighth_note()
{
    SequencerPage current_page = _pages[_current_page_index];
    
    if (!current_page.has_playable_eighth_notes_after(_current_eighth_note_index))
    {
        next_page();
        current_page = _pages[_current_page_index];
        _current_eighth_note_index = current_page.get_first_playable_eighth_note_index();
    }
    else
    {
        _current_eighth_note_index = current_page.get_first_playable_eighth_note_index_after(_current_eighth_note_index);
    }
}

void Sequencer::next_page()
{
    uint8_t next_page_index = _current_page_index + 1;
    for (; next_page_index < SEQUENCER_PAGES_NUM; next_page_index++)
    {
        if (_played_pages[next_page_index] && _pages[next_page_index].has_playable_eighth_notes())
        {
            break;
        }
    }


    if (next_page_index >= SEQUENCER_PAGES_NUM)
    {
        for (next_page_index = 0; next_page_index < _current_page_index; next_page_index++)
        {
            if (_played_pages[next_page_index] && _pages[next_page_index].has_playable_eighth_notes())
            {
                break;
            }
        }
    }

    if (next_page_index == _current_page_index)
    {
        next_page_index = _edited_page_index;
    }
    
    _current_page_index = next_page_index;
}

void Sequencer::_set_playing_from_play_pause_switch(bool play_pause_switch_pushed)
{
    if (!_is_playing && play_pause_switch_pushed)
    {
        _start_playing();
    }
    else if (_is_playing && !play_pause_switch_pushed)
    {
        _stop_playing();
    }
}

void Sequencer::_start_playing()
{
    _is_playing = true;
    _sequencer_callback(SEQUENCER_CB_START_PLAYING, NULL, _sequencer_callback_context);
}

void Sequencer::_stop_playing()
{
    _current_page_index = _get_first_played_page_index();
    _current_eighth_note_index = 0;
    _is_playing = false;
    _sequencer_callback(SEQUENCER_CB_STOP_PLAYING, NULL, _sequencer_callback_context);
}

uint8_t Sequencer::_get_first_played_page_index()
{
    for (uint8_t i = 0; i < SEQUENCER_PAGES_NUM; i++)
    {
        if (_played_pages[i])
        {
            return i;
        }
    }
    
    return 0;
}

// Sets BPM from potentiometer value. Potentiometer value is 0 < value < 1.
void Sequencer::_set_bpm_from_potentiometer(const float potentiometer_value)
{
    float new_bpm = potentiometer_value * (SEQUENCER_BPM_MAX - SEQUENCER_BPM_MIN) + SEQUENCER_BPM_MIN;

    if (abs(new_bpm - _bpm) > SEQUENCER_BPM_CHANGE_MIN)
    {
        _bpm = new_bpm;
        _sequencer_callback(SEQUENCER_CB_BPM_CHANGE, NULL, _sequencer_callback_context);
    }
}

void Sequencer::_set_tracks_enabled_from_push_buttons(const push_button_event_t *push_buttons_events)
{
    for (size_t i = 0; i < SEQUENCER_TRACKS_NUM; i++)
    {
        uint8_t click_events_pending = push_buttons_events[i].click_events_pending;
        // If odd number of clicks toggle track state
        if (click_events_pending % 2 != 0)
        {
            _tracks_enabled[i] = !_tracks_enabled[i];
        }
    }
}


inline measure_state_t _rotary_button_state_to_measure_state(const rotary_button_state_t rotary_button_state)
{
    switch (rotary_button_state)
    {
        case ROTARY_BUTTON_SKIP:
            return MEASURE_STATE_SKIP;
        case ROTARY_BUTTON_PLAY:
            return MEASURE_STATE_PLAY;
        case ROTARY_BUTTON_LOCK:
            return MEASURE_STATE_LOCK;
        default:
            return MEASURE_STATE_UNKNOWN;
    }
}

void Sequencer::_set_measures_states_from_rotary_buttons(const rotary_button_state_t *rotary_buttons_states)
{
    for (size_t i = 0; i < SEQUENCER_MEASURES_NUM; i++)
    {
        measure_state_t measure_state = _rotary_button_state_to_measure_state(rotary_buttons_states[i]);
        if (measure_state != MEASURE_STATE_UNKNOWN)
        {
            _measures_states[i] = measure_state;
        }
    }
}
