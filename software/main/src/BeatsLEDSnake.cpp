#include "BeatsLEDSnake.h"

#include <freertos/FreeRTOS.h>
#include <esp_timer.h>


static void _led_snake_timer_callback(void *led_snake_arg)
{
    BeatsLEDSnake *led_snake = (BeatsLEDSnake *)led_snake_arg;
    led_snake->update_next_led();
}

BeatsLEDSnake::BeatsLEDSnake() : _mux(BEATS_SNAKE_MUX_S0_PIN, BEATS_SNAKE_MUX_S1_PIN, BEATS_SNAKE_MUX_S2_PIN, BEATS_SNAKE_MUX_S3_PIN)
{
    _last_updated_led = 15;

    gpio_reset_pin(BEATS_SNAKE_LED_POWER_PIN);
    gpio_set_direction(BEATS_SNAKE_LED_POWER_PIN, GPIO_MODE_OUTPUT);
    disable_all_leds();

    _init_timer();
}

void BeatsLEDSnake::_init_timer()
{
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &_led_snake_timer_callback,
        .arg = this,
        .name = "BeatsLEDSnake"};

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, BEATS_SNAKE_TIMER_PERIOD));
}

void BeatsLEDSnake::enable_led(uint8_t index)
{
    if (index > 15)
    {
        return;
    }

    _enabled_leds[index] = true;
}

void BeatsLEDSnake::disable_led(uint8_t index)
{
    if (index > 15)
    {
        return;
    }

    _enabled_leds[index] = false;
}

void BeatsLEDSnake::enable_all_leds()
{
    for (size_t i = 0; i < 16; i++)
    {
        _enabled_leds[i] = true;
    }
}

void BeatsLEDSnake::disable_all_leds()
{
    for (size_t i = 0; i < 16; i++)
    {
        _enabled_leds[i] = false;
    }
}

void BeatsLEDSnake::update_next_led()
{
    uint8_t led_index = _last_updated_led + 1;

    if (led_index > 15) {
        led_index = 0;
    }

    _mux.channel(led_index);
    gpio_set_level(BEATS_SNAKE_LED_POWER_PIN, _enabled_leds[led_index]);

    _last_updated_led = led_index;
}
