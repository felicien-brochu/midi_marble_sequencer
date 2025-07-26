#include "LEDColumn.h"

LEDColumn::LEDColumn() {
    // Init LED gpios
    for (size_t i = 0; i < SEQUENCER_TRACKS_NUM; i++)
    {
        gpio_num_t gpio = _led_gpios[i];
        gpio_reset_pin(gpio);
        gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
        enable_all_leds();
    }
}

void LEDColumn::update_leds(bool *leds_enabled)
{
    for (size_t i = 0; i < SEQUENCER_TRACKS_NUM; i++)
    {
        gpio_num_t gpio = _led_gpios[i];
        gpio_set_level(gpio, !leds_enabled[i]);
    }
    
}

void LEDColumn::enable_led(uint8_t index)
{
    if (index >= SEQUENCER_TRACKS_NUM)
    {
        return;
    }
    
    gpio_num_t gpio = _led_gpios[index];
    gpio_set_level(gpio, false);
}

void LEDColumn::disable_led(uint8_t index)
{
    if (index >= SEQUENCER_TRACKS_NUM)
    {
        return;
    }

    gpio_num_t gpio = _led_gpios[index];
    gpio_set_level(gpio, true);
}

void LEDColumn::enable_all_leds()
{
    for (size_t i = 0; i < SEQUENCER_TRACKS_NUM; i++)
    {
        gpio_num_t gpio = _led_gpios[i];
        gpio_set_level(gpio, false);
    }
}

void LEDColumn::disable_all_leds()
{
    for (size_t i = 0; i < SEQUENCER_TRACKS_NUM; i++)
    {
        gpio_num_t gpio = _led_gpios[i];
        gpio_set_level(gpio, true);
    }
}
