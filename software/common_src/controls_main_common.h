#pragma once

#include "sequencer_config.h"

#include <stdint.h>

typedef enum
{
    ROTARY_BUTTON_UNKNOWN = 0,
    ROTARY_BUTTON_SKIP,
    ROTARY_BUTTON_PLAY,
    ROTARY_BUTTON_LOCK,
    ROTARY_BUTTON_MAX
} rotary_button_state_t;

typedef struct {
    bool pushed;
    uint8_t click_events_pending;
} push_button_event_t;

typedef struct
{
    float bpm_potentiometer_value;
    bool play_pause_switch_pushed;
    rotary_button_state_t rotary_buttons_states[SEQUENCER_MEASURES_NUM];
    push_button_event_t tracks_push_buttons[SEQUENCER_TRACKS_NUM];
    uint16_t crc16;
} controls_main_value_t;


typedef struct 
{
    bool play_pause_led_enabled;
    bool tracks_led_enabled[SEQUENCER_TRACKS_NUM];
    uint16_t crc16;
} controls_main_display_t;
