#include "LEDRow.h"


LEDRow::LEDRow(LEDArray &led_array, uint8_t num_leds, uint8_t *led_indexes) : _led_array(led_array)
{
    _num_leds = num_leds;
    _led_indexes = led_indexes;
}


void LEDRow::enable_led(uint8_t index)
{
    if (index >= _num_leds)
    {
        return;
    }

    _led_array.enable_led(_led_indexes[index]);
}

void LEDRow::disable_led(uint8_t index)
{
    if (index >= _num_leds)
    {
        return;
    }

    _led_array.disable_led(_led_indexes[index]);
}

void LEDRow::enable_all_leds()
{
    for (size_t i = 0; i < _num_leds; i++)
    {
        _led_array.enable_led(_led_indexes[i]);
    }
}

void LEDRow::disable_all_leds()
{
    for (size_t i = 0; i < _num_leds; i++)
    {
        _led_array.disable_led(_led_indexes[i]);
    }
}
