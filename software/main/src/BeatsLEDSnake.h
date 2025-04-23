#pragma once

#include <driver/gpio.h>
#include <CD74HC4067.h>

#define BEATS_SNAKE_LED_POWER_PIN GPIO_NUM_10

#define BEATS_SNAKE_MUX_S0_PIN    GPIO_NUM_14
#define BEATS_SNAKE_MUX_S1_PIN    GPIO_NUM_13
#define BEATS_SNAKE_MUX_S2_PIN    GPIO_NUM_12
#define BEATS_SNAKE_MUX_S3_PIN    GPIO_NUM_11

#define BEATS_SNAKE_TIMER_PERIOD 800 //> Period of the timer in microseconds

class BeatsLEDSnake
{
public:
    BeatsLEDSnake();


    void enable_led(uint8_t index);
    void disable_led(uint8_t index);
    void enable_all_leds();
    void disable_all_leds();

    void update_next_led();

private:
    CD74HC4067 _mux;
    uint8_t _last_updated_led;

    bool _enabled_leds[16];


    void _init_timer();
};