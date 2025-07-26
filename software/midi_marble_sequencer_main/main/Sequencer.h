#pragma once

#include "sequencer_config.h"
#include "controls_main_common.h"


#define SEQUENCER_BPM_MIN 10
#define SEQUENCER_BPM_MAX 600

typedef enum {
    MEASURE_STATE_UNKNOWN = 0,
    MEASURE_STATE_SKIP,
    MEASURE_STATE_PLAY,
    MEASURE_STATE_LOCK,
    MEASURE_STATE_MAX
} measure_state_t;


class Sequencer
{
public:
    Sequencer();

    controls_main_display_t get_controls_main_display();
    void handle_controls_main_event(const controls_main_value_t controls_main_value);

private:
    bool _is_playing;
    float _bpm;
    bool _tracks_enabled[SEQUENCER_TRACKS_NUM];
    measure_state_t _measures_states[SEQUENCER_MEASURES_NUM];


    void _set_bpm_from_potentiometer(const float potentiometer_value);
    void _set_tracks_enabled_from_push_buttons(const push_button_event_t *push_buttons_events);
    void _set_measures_states_from_rotary_buttons(const rotary_button_state_t *rotary_buttons_states);
};