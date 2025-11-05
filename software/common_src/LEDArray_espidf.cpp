#include "LEDArray_espidf.h"

#include <freertos/FreeRTOS.h>
#include <esp_timer.h>
#include <esp_log.h>


static void _led_array_timer_callback(void *led_array_arg)
{
    LEDArray *led_array = (LEDArray *)led_array_arg;
    led_array->update_next_led();
}

LEDArray::LEDArray(uint8_t num_leds, gpio_num_t power_gpio, gpio_num_t s0_gpio, gpio_num_t s1_gpio, gpio_num_t s2_gpio, gpio_num_t s3_gpio) : _mux(s0_gpio, s1_gpio, s2_gpio, s3_gpio)
{
    _num_leds = num_leds;
    _power_gpio = power_gpio;
    _last_updated_led = _num_leds - 1;

    gpio_reset_pin(_power_gpio);
    gpio_set_direction(_power_gpio, GPIO_MODE_OUTPUT);
    disable_all_leds();

    _init_timer();
}

void LEDArray::_init_timer()
{
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &_led_array_timer_callback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "LEDArray"};

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LED_ARRAY_TIMER_PERIOD));
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
    gpio_set_level(_power_gpio, _enabled_leds[led_index]);

    _last_updated_led = led_index;
}
