#pragma once

#include "sequencer_config.h"
#include "controls_common.h"

#include <cstdint>

typedef struct
{
    uint16_t played_pages_buttons;
    uint16_t edited_pages_buttons;
    uint16_t crc16;
} controls_pages_value_t;


typedef struct 
{
    bool played_pages_led_enabled[SEQUENCER_PAGES_NUM];
    bool edited_pages_led_enabled[SEQUENCER_PAGES_NUM];
    uint8_t played_pages_buttons_clicks_consumed;
    uint8_t edited_pages_buttons_clicks_consumed;
    uint16_t crc16;
} controls_pages_display_t;
