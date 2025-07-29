#pragma once

#include <driver/gpio.h>
#include "CD74HC4067.h"

#define LED_ARRAY_TIMER_PERIOD 800 //> Period of the timer in microseconds
#define LED_ARRAY_LEDS_MAX_NUM 16

class LEDArray
{
public:
    LEDArray(uint8_t num_leds, gpio_num_t power_gpio, gpio_num_t s0_gpio, gpio_num_t s1_gpio, gpio_num_t s2_gpio, gpio_num_t s3_gpio);


    void enable_led(uint8_t index);
    void disable_led(uint8_t index);
    void enable_all_leds();
    void disable_all_leds();

    void update_next_led();

private:
    uint8_t _num_leds;
    gpio_num_t _power_gpio;
    
    CD74HC4067 _mux;
    uint8_t _last_updated_led;

    bool _enabled_leds[LED_ARRAY_LEDS_MAX_NUM];


    void _init_timer();
};