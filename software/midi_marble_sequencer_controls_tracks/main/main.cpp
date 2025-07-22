#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>

#include "LEDRow.h"
#include "LEDArray.h"

#define LED_ARRAY_POWER_GPIO GPIO_NUM_15

#define LED_ARRAY_S0_GPIO GPIO_NUM_17
#define LED_ARRAY_S1_GPIO GPIO_NUM_16
#define LED_ARRAY_S2_GPIO GPIO_NUM_4
#define LED_ARRAY_S3_GPIO GPIO_NUM_2


void main_test_led_row()
{
    LEDArray led_array(16, LED_ARRAY_POWER_GPIO, LED_ARRAY_S0_GPIO, LED_ARRAY_S1_GPIO, LED_ARRAY_S2_GPIO, LED_ARRAY_S3_GPIO);

    uint8_t playing_led_indexes[] = {7, 6, 5, 4, 3, 2, 1, 0};
    LEDRow playing_led_row(led_array, 8, playing_led_indexes);

    uint8_t editing_led_indexes[] = {8, 9, 10, 11, 12, 13, 14, 15};
    LEDRow editing_led_row(led_array, 8, editing_led_indexes);

    while (1)
    {
        for (size_t i = 0; i < 8; i++)
        {
            playing_led_row.disable_all_leds();
            editing_led_row.disable_all_leds();
            playing_led_row.enable_led(i);
            editing_led_row.enable_led(i);
            vTaskDelay(pdMS_TO_TICKS(600));
        }
    }
}

extern "C" void app_main()
{
    esp_log_level_set("*", ESP_LOG_DEBUG);

    main_test_led_row();
}