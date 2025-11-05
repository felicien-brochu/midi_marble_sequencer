#pragma once

#include "controls_main_common.h"
#include "CD74HC4067_arduino.h"

#define ROTARY_BUTTONS_MULTISAMPLING 4
#define ROTARY_BUTTONS_NUM SEQUENCER_MEASURES_NUM

// Thresholds to determine state of the rotary buttons.
// Typical adc values for each state:
//   * ROTARY_BUTTON_UNKNOWN: 0
//   * ROTARY_BUTTON_SKIP: 1830
//   * ROTARY_BUTTON_PLAY: 2480
//   * ROTARY_BUTTON_LOCK: 4095

#define ROTARY_BUTTONS_THRESHOLD_SKIP 915
#define ROTARY_BUTTONS_THRESHOLD_PLAY 2155
#define ROTARY_BUTTONS_THRESHOLD_LOCK 3288

class RotaryButtonsController
{
public:
    RotaryButtonsController(CD74HC4067 &mux, uint8_t adc_gpio);

    void update();
    rotary_button_state_t *get_rotary_buttons_states();
    void consume_events();

private:    
    CD74HC4067 &_mux;
    uint8_t _adc_gpio;
    uint16_t _adc_raw;

    rotary_button_state_t _rotary_buttons_states[ROTARY_BUTTONS_NUM];
    uint8_t _state_change_events_pending[ROTARY_BUTTONS_NUM];

    rotary_button_state_t _read_state_from_adc(uint8_t rotary_button_index);
    uint16_t _read_adc_value(uint8_t rotary_button_index);
};