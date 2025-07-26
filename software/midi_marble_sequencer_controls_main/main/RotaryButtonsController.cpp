#include "RotaryButtonsController.h"

RotaryButtonsController::RotaryButtonsController(adc_oneshot_unit_handle_t &adc_handle, CD74HC4067 &mux, adc_channel_t adc_channel) : _adc_handle(adc_handle), _mux(mux), _adc_channel(adc_channel)
{
    for (size_t i = 0; i < ROTARY_BUTTONS_NUM; i++)
    {
        _rotary_buttons_states[i] = ROTARY_BUTTON_UNKNOWN;
        _state_change_events_pending[i] = 0;
    }


    adc_oneshot_chan_cfg_t adc_channel_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(_adc_handle, _adc_channel, &adc_channel_config));
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
    int adc_value = _read_adc_value(rotary_button_index);

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

int RotaryButtonsController::_read_adc_value(uint8_t rotary_button_index)
{
    _mux.channel(rotary_button_index);

    int adc_raw_sum = 0;
    for (size_t i = 0; i < ROTARY_BUTTONS_MULTISAMPLING; i++)
    {
        ESP_ERROR_CHECK(adc_oneshot_read(_adc_handle, _adc_channel, _adc_raw));
        adc_raw_sum += _adc_raw[0];
    }

    int adc_mean_value = adc_raw_sum / ROTARY_BUTTONS_MULTISAMPLING;
    return adc_mean_value;
}