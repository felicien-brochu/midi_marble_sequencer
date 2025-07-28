#include "BeatsLEDSnake.h"

BeatsLEDSnake::BeatsLEDSnake() : LEDArray(BEATS_SNAKE_NUM_LEDS, BEATS_SNAKE_LED_POWER_PIN, BEATS_SNAKE_MUX_S0_PIN, BEATS_SNAKE_MUX_S1_PIN, BEATS_SNAKE_MUX_S2_PIN, BEATS_SNAKE_MUX_S3_PIN)
{
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
    disable_all_leds();

    if (_enabled) {
        uint8_t led_index = _eighth_note_index / 2;
        enable_led(led_index);
    }
}