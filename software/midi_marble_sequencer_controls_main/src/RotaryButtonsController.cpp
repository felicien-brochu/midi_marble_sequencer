#include "RotaryButtonsController.h"
#include <Arduino.h>

RotaryButtonsController::RotaryButtonsController(CD74HC4067 &mux, uint8_t adc_gpio) : _mux(mux), _adc_gpio(adc_gpio)
{
    for (size_t i = 0; i < ROTARY_BUTTONS_NUM; i++)
    {
        _rotary_buttons_states[i] = ROTARY_BUTTON_UNKNOWN;
        _state_change_events_pending[i] = 0;
    }

    // analogSetAttenuation(ADC_ATTENDB_MAX);
    // analogSetWidth(12);
}

void RotaryButtonsController::update()
{
    for (size_t rotary_button_index = 0; rotary_button_index < ROTARY_BUTTONS_NUM; rotary_button_index++)
    {
        rotary_button_state_t rotary_button_state = _read_state_from_adc(rotary_button_index);

        if (rotary_button_state != _rotary_buttons_states[rotary_button_index])
        {
            _rotary_buttons_states[rotary_button_index] = rotary_button_state;
            _state_change_events_pending[rotary_button_index]++;
        }
    }
}

rotary_button_state_t *RotaryButtonsController::get_rotary_buttons_states()
{
    return _rotary_buttons_states;
}

void RotaryButtonsController::consume_events()
{
    for (size_t i = 0; i < ROTARY_BUTTONS_NUM; i++)
    {
        _state_change_events_pending[i] = 0;
    }
}

rotary_button_state_t RotaryButtonsController::_read_state_from_adc(uint8_t rotary_button_index)
{
    uint16_t adc_value = _read_adc_value(rotary_button_index);

    rotary_button_state_t button_state = ROTARY_BUTTON_UNKNOWN;

    if (adc_value >= ROTARY_BUTTONS_THRESHOLD_LOCK) {
        button_state = ROTARY_BUTTON_LOCK;
    }
    else if (adc_value >= ROTARY_BUTTONS_THRESHOLD_PLAY) {
        button_state = ROTARY_BUTTON_PLAY;
    }
    else if (adc_value >= ROTARY_BUTTONS_THRESHOLD_SKIP) {
        button_state = ROTARY_BUTTON_SKIP;
    }

    return button_state;
}

uint16_t RotaryButtonsController::_read_adc_value(uint8_t rotary_button_index)
{
    _mux.channel(rotary_button_index);

    uint16_t adc_raw_sum = 0;
    for (size_t i = 0; i < ROTARY_BUTTONS_MULTISAMPLING; i++)
    {
        // _adc_raw = 1000;
        _adc_raw = analogRead(_adc_gpio);
        adc_raw_sum += _adc_raw;
    }

    uint16_t adc_mean_value = adc_raw_sum / ROTARY_BUTTONS_MULTISAMPLING;
    return adc_mean_value;
}