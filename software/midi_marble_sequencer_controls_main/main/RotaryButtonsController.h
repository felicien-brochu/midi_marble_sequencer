#pragma once

#include "CD74HC4067.h"

#include <esp_adc/adc_oneshot.h>

#define ROTARY_BUTTONS_NUM 4
#define ROTARY_BUTTONS_MULTISAMPLING 4

// Thresholds to determine state of the rotary buttons.
// Typical adc values for each state:
//   * ROTARY_BUTTON_UNKNOWN: 0
//   * ROTARY_BUTTON_SKIP: 1830
//   * ROTARY_BUTTON_PLAY: 2480
//   * ROTARY_BUTTON_LOCK: 4095

#define ROTARY_BUTTONS_THRESHOLD_SKIP 915
#define ROTARY_BUTTONS_THRESHOLD_PLAY 2155
#define ROTARY_BUTTONS_THRESHOLD_LOCK 3288

typedef enum {
    ROTARY_BUTTON_UNKNOWN = 0,
    ROTARY_BUTTON_SKIP,
    ROTARY_BUTTON_PLAY,
    ROTARY_BUTTON_LOCK,
    ROTARY_BUTTON_MAX
} rotary_button_state_t;

class RotaryButtonsController
{
public:
    RotaryButtonsController(adc_oneshot_unit_handle_t &adc_handle, CD74HC4067 &mux, adc_channel_t adc_channel);

    void update();

private:
    adc_oneshot_unit_handle_t &_adc_handle;
    
    CD74HC4067 &_mux;
    adc_channel_t _adc_channel;
    
    int _adc_raw[1];

    rotary_button_state_t _button_states[ROTARY_BUTTONS_NUM];
    uint8_t _state_change_events_pending[ROTARY_BUTTONS_NUM];

    rotary_button_state_t _read_state_from_adc(uint8_t rotary_button_index);
    int _read_adc_value(uint8_t rotary_button_index);
};