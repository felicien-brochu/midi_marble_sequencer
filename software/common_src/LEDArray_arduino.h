#pragma once

#include "CD74HC4067_arduino.h"

#include "esp32-hal-timer.h"

#define LED_ARRAY_TIMER_PERIOD 800 //> Period of the timer in us
#define LED_ARRAY_LEDS_MAX_NUM 16

class LEDArray
{
public:
    LEDArray(uint8_t num_leds, uint8_t power_gpio, uint8_t s0_gpio, uint8_t s1_gpio, uint8_t s2_gpio, uint8_t s3_gpio);

    static LEDArray *instance;

    void enable_led(uint8_t index);
    void disable_led(uint8_t index);
    void enable_all_leds();
    void disable_all_leds();

    void update_next_led();

private:
    uint8_t _num_leds;
    uint8_t _power_gpio;
    
    CD74HC4067 _mux;
    uint8_t _last_updated_led;

    bool _enabled_leds[LED_ARRAY_LEDS_MAX_NUM];

    hw_timer_t *_led_array_timer;


    void _init_timer();
};