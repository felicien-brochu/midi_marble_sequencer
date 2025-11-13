#pragma once

#include "sequencer_config.h"

#include <cstdint>

typedef enum
{
    LED_DISPLAY_STATE_OFF = 0,
    LED_DISPLAY_STATE_ON,
    LED_DISPLAY_STATE_BLINK,
    LED_DISPLAY_STATE_NUM,
} led_display_state_t;

typedef struct
{
    uint16_t played_pages_buttons;
    uint16_t edited_pages_buttons;
    uint16_t crc16;
} controls_pages_value_t;


typedef struct 
{
    led_display_state_t played_pages_led_states[SEQUENCER_PAGES_NUM];
    led_display_state_t edited_pages_led_states[SEQUENCER_PAGES_NUM];
    uint8_t played_pages_buttons_clicks_consumed;
    uint8_t edited_pages_buttons_clicks_consumed;
    uint16_t crc16;
} controls_pages_display_t;
