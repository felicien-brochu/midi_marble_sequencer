#pragma once

#include "BlinkClock.h"

#include <LEDArray_arduino.h>
#include <controls_pages_common.h>

class LEDRow : public IBlinkClockListener
{
public:
    LEDRow(LEDArray &led_array, uint8_t num_leds, const uint8_t *led_indexes);

    void set_led_states(led_display_state_t *led_states);
    void on_blink_clock_tick(bool blink_is_on);

private:
    LEDArray &_led_array;
    uint8_t _num_leds;
    const uint8_t *_led_indexes;

    led_display_state_t *_led_states;
};