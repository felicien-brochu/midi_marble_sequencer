#include "Sequencer.h"

#include <cstring>
#include <cstddef>

Sequencer::Sequencer()
{
    _is_playing = false;
    _bpm = 100;

    for (size_t i = 0; i < SEQUENCER_TRACKS_NUM; i++)
    {
        _tracks_enabled[i] = true;
    }

    for (size_t i = 0; i < SEQUENCER_MEASURES_NUM; i++)
    {
        _measures_states[i] = MEASURE_STATE_PLAY;
    }
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
    _is_playing = controls_main_value.play_pause_switch_pushed;
    _set_bpm_from_potentiometer(controls_main_value.bpm_potentiometer_value);
    _set_tracks_enabled_from_push_buttons(controls_main_value.tracks_push_buttons);
    _set_measures_states_from_rotary_buttons(controls_main_value.rotary_buttons_states);
}

// Sets BPM from potentiometer value. Potentiometer value is 0 < value < 1.
void Sequencer::_set_bpm_from_potentiometer(const float potentiometer_value)
{
    _bpm = potentiometer_value * (SEQUENCER_BPM_MAX - SEQUENCER_BPM_MIN) + SEQUENCER_BPM_MIN;
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
