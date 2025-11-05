#pragma once

#include "sequencer_config.h"
#include "controls_common.h"

#include <cstdint>

typedef struct
{
    push_button_event_t played_pages_buttons[SEQUENCER_PAGES_NUM];
    push_button_event_t edited_pages_buttons[SEQUENCER_PAGES_NUM];
    uint16_t crc16;
} controls_pages_value_t;


typedef struct 
{
    bool played_pages_led_enabled[SEQUENCER_PAGES_NUM];
    bool edited_pages_led_enabled[SEQUENCER_PAGES_NUM];
    uint16_t crc16;
} controls_pages_display_t;
