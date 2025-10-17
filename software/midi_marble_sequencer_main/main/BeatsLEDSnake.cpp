#include "BeatsLEDSnake.h"

#include <driver/gpio.h>

BeatsLEDSnake::BeatsLEDSnake() : _mux(BEATS_SNAKE_MUX_S0_PIN, BEATS_SNAKE_MUX_S1_PIN, BEATS_SNAKE_MUX_S2_PIN, BEATS_SNAKE_MUX_S3_PIN)
{
    _enabled = false;
    _eighth_note_index = 0;

    gpio_set_direction(BEATS_SNAKE_LED_POWER_PIN, GPIO_MODE_OUTPUT);

    update();
}


void BeatsLEDSnake::set_eighth_note_index(uint8_t eighth_note_index)
{
    _eighth_note_index = eighth_note_index;
}

void BeatsLEDSnake::set_enabled(bool enabled)
{
    _enabled = enabled;
}

void BeatsLEDSnake::update()
{
    if (_enabled) {
        uint8_t led_index = _eighth_note_index / 2;
        _mux.channel(led_index);
        gpio_set_level(BEATS_SNAKE_LED_POWER_PIN, 1);
    }
    else {
        gpio_set_level(BEATS_SNAKE_LED_POWER_PIN, 0);
    }
}