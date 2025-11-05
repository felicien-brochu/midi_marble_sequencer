#pragma once

#include <LEDArray.h>

class LEDRow
{
public:
    LEDRow(LEDArray &led_array, uint8_t num_leds, const uint8_t *led_indexes);

    void enable_led(uint8_t index);
    void disable_led(uint8_t index);
    void enable_all_leds();
    void disable_all_leds();

private:
    LEDArray &_led_array;
    uint8_t _num_leds;
    const uint8_t *_led_indexes;
};