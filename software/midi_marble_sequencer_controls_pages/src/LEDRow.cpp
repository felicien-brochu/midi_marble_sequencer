#include "LEDRow.h"


LEDRow::LEDRow(LEDArray &led_array, uint8_t num_leds, const uint8_t *led_indexes) : _led_array(led_array)
{
    _num_leds = num_leds;
    _led_indexes = led_indexes;
    _led_states = (led_display_state_t *) malloc(_num_leds * sizeof(led_display_state_t));

    BlinkClock::instance().add_listener(this);
}

void LEDRow::set_led_states(led_display_state_t *led_states)
{
    for (size_t i = 0; i < _num_leds; i++)
    {
        if (led_states[i] == LED_DISPLAY_STATE_ON)
        {
            _led_array.enable_led(_led_indexes[i]);
        }
        else if (led_states[i] == LED_DISPLAY_STATE_OFF)
        {
            _led_array.disable_led(_led_indexes[i]);
        }
        else if (led_states[i] == LED_DISPLAY_STATE_BLINK && _led_states[i] != LED_DISPLAY_STATE_BLINK)
        {
            _led_array.disable_led(_led_indexes[i]);
            BlinkClock::instance().restart_blink();
        }
    }

    memcpy(_led_states, led_states, _num_leds * sizeof(led_display_state_t));
}

void LEDRow::on_blink_clock_tick(bool blink_is_on)
{
    for (size_t i = 0; i < _num_leds; i++)
    {
        if (_led_states[i] == LED_DISPLAY_STATE_BLINK)
        {
            if (blink_is_on)
            {
                _led_array.enable_led(_led_indexes[i]);
            }
            else
            {
                _led_array.disable_led(_led_indexes[i]);
            }
        }
    } 
}
