#include "LEDColumn.h"
#include <Arduino.h>

LEDColumn::LEDColumn() {
    // Init LED gpios
    for (size_t i = 0; i < SEQUENCER_TRACKS_NUM; i++)
    {
        uint8_t gpio = _led_gpios[i];
        pinMode(gpio, OUTPUT);
        enable_all_leds();
    }
}

void LEDColumn::update_leds(bool *leds_enabled)
{
    for (size_t i = 0; i < SEQUENCER_TRACKS_NUM; i++)
    {
        if (!_led_gpios[i])
        {
            // ESP_LOGI("debug ledcolumn", "led[%d] disabled", i);
        }
        uint8_t gpio = _led_gpios[i];
        digitalWrite(gpio, !leds_enabled[i]);
    }
    
}

void LEDColumn::enable_led(uint8_t index)
{
    if (index >= SEQUENCER_TRACKS_NUM)
    {
        return;
    }
    
    uint8_t gpio = _led_gpios[index];
    digitalWrite(gpio, false);
}

void LEDColumn::disable_led(uint8_t index)
{
    if (index >= SEQUENCER_TRACKS_NUM)
    {
        return;
    }

    uint8_t gpio = _led_gpios[index];
    digitalWrite(gpio, true);
}

void LEDColumn::enable_all_leds()
{
    for (size_t i = 0; i < SEQUENCER_TRACKS_NUM; i++)
    {
        uint8_t gpio = _led_gpios[i];
        digitalWrite(gpio, false);
    }
}

void LEDColumn::disable_all_leds()
{
    for (size_t i = 0; i < SEQUENCER_TRACKS_NUM; i++)
    {
        uint8_t gpio = _led_gpios[i];
        digitalWrite(gpio, true);
    }
}
