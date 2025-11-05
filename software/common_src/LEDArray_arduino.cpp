#include "LEDArray_arduino.h"

#include <Arduino.h>


LEDArray *LEDArray::instance;

IRAM_ATTR void _led_array_timer_callback()
{
    LEDArray::instance->update_next_led();
}

LEDArray::LEDArray(uint8_t num_leds, uint8_t power_gpio, uint8_t s0_gpio, uint8_t s1_gpio, uint8_t s2_gpio, uint8_t s3_gpio) : _mux(s0_gpio, s1_gpio, s2_gpio, s3_gpio)
{
    LEDArray::instance = this;
    _num_leds = num_leds;
    _power_gpio = power_gpio;
    _last_updated_led = _num_leds - 1;

    pinMode(_power_gpio, OUTPUT);
    disable_all_leds();

    _init_timer();
}

void LEDArray::_init_timer()
{
    _led_array_timer = timerBegin(1, 80, true);
    timerAttachInterrupt(_led_array_timer, &_led_array_timer_callback, false);
    timerAlarmWrite(_led_array_timer, LED_ARRAY_TIMER_PERIOD, true);
    timerAlarmEnable(_led_array_timer);
}

void LEDArray::enable_led(uint8_t index)
{
    if (index >= _num_leds)
    {
        return;
    }

    _enabled_leds[index] = true;
}

void LEDArray::disable_led(uint8_t index)
{
    if (index >= _num_leds)
    {
        return;
    }

    _enabled_leds[index] = false;
}

void LEDArray::enable_all_leds()
{
    for (size_t i = 0; i < _num_leds; i++)
    {
        _enabled_leds[i] = true;
    }
}

void LEDArray::disable_all_leds()
{
    for (size_t i = 0; i < _num_leds; i++)
    {
        _enabled_leds[i] = false;
    }
}

void LEDArray::update_next_led()
{
    uint8_t led_index = _last_updated_led + 1;
    
    if (led_index >= _num_leds) {
        led_index = 0;
    }
    
    _mux.channel(led_index);
    digitalWrite(_power_gpio, _enabled_leds[led_index]);

    _last_updated_led = led_index;
}
