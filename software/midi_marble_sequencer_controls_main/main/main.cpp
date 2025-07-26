#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_err.h>

#include "BPMPotentiometer.h"
#include "I2CSlaveController.h"
#include "LEDColumn.h"
#include "PlayPauseLED.h"
#include "PlayPauseSwitch.h"
#include "PushButtonsController.h"
#include "RotaryButtonsController.h"
#include "CD74HC4067.h"
#include "InteractionController.h"
#include "DisplayController.h"


void i2c_slave_controller_task(void *i2c_slave_controller_arg)
{
    I2CSlaveController *i2c_slave_controller = (I2CSlaveController *)i2c_slave_controller_arg;
    i2c_slave_controller->main_task();
}


void interaction_controller_task(void *interaction_controller_arg)
{
    InteractionController *interaction_controller = (InteractionController *)interaction_controller_arg;

    while (true)
    {
        interaction_controller->update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void main_controls_main()
{
    InteractionController interaction_controller;
    DisplayController display_controller;

    I2CSlaveController i2c_slave_controller(interaction_controller, display_controller);
    TaskHandle_t i2c_task_handle;
    xTaskCreate(i2c_slave_controller_task, "I2CSlaveController", 10000, &i2c_slave_controller, 3, &i2c_task_handle);

    // TaskHandle_t interaction_task_handle;
    // xTaskCreate(interaction_controller_task, "InteractionController", 10000, &interaction_controller, 3, &interaction_task_handle);

    while (true)
    {
        interaction_controller.update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void main_test_leds()
{
    LEDColumn led_column;
    PlayPauseLED play_pause_led;

    while (1)
    {
        for (size_t i = 0; i < 8; i++)
        {
            vTaskDelay(pdMS_TO_TICKS(600));
            if (i % 2 == 0) {
                play_pause_led.enable();
            }
            else {
                play_pause_led.disable();
            }
            led_column.disable_all_leds();
            led_column.enable_led(i);
        }
    }
}

// void main_test_buttons()
// {
//     adc_oneshot_unit_handle_t adc_handle;

//     adc_oneshot_unit_init_cfg_t adc_init_config = {
//         .unit_id = BPM_POT_ADC_UNIT,
//     };

//     ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_init_config, &adc_handle));
    
//     BPMPotentiometer bpm_potentiometer(adc_handle);
//     PlayPauseSwitch play_pause_switch;

//     CD74HC4067 mux(BUTTONS_MUX_S0_GPIO, BUTTONS_MUX_S1_GPIO, BUTTONS_MUX_S2_GPIO, BUTTONS_MUX_S3_GPIO);

//     RotaryButtonsController rotary_buttons_controller(adc_handle, mux, BUTTONS_MUX_ADC_CHANNEL);
//     // PushButtonsController must be initialized after RotaryButtonsController because of GPIO settings order.
//     PushButtonsController push_buttons_controller(mux, BUTTONS_MUX_SIGNAL_GPIO);

//     while(1) {
//         play_pause_switch.update();
//         bpm_potentiometer.update();
//         push_buttons_controller.update();
//         rotary_buttons_controller.update();
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// }

// void main_print_adc_value()
// {
//     gpio_reset_pin(BUTTONS_MUX_SIGNAL_GPIO);
//     gpio_set_direction(BUTTONS_MUX_SIGNAL_GPIO, GPIO_MODE_INPUT);
//     gpio_pullup_dis(BUTTONS_MUX_SIGNAL_GPIO);
//     gpio_pulldown_dis(BUTTONS_MUX_SIGNAL_GPIO);

//     CD74HC4067 mux(BUTTONS_MUX_S0_GPIO, BUTTONS_MUX_S1_GPIO, BUTTONS_MUX_S2_GPIO, BUTTONS_MUX_S3_GPIO);

//     mux.channel(3);

//     adc_unit_t adc_unit = BUTTONS_MUX_ADC_UNIT;
//     adc_channel_t adc_channel = BUTTONS_MUX_ADC_CHANNEL;
//     adc_oneshot_unit_handle_t adc_handle;

//     adc_oneshot_chan_cfg_t adc_channel_config = {
//         .atten = ADC_ATTEN_DB_12,
//         .bitwidth = ADC_BITWIDTH_DEFAULT,
//     };

//     adc_oneshot_unit_init_cfg_t adc_init_config = {
//         .unit_id = adc_unit,
//     };

//     ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_init_config, &adc_handle));
//     ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, adc_channel, &adc_channel_config));

//     int adc_raw[1];

//     while(1) {
//         ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, adc_channel, adc_raw));

//         printf(">RotaryButton0: %d\n", adc_raw[0]);
//         vTaskDelay(pdMS_TO_TICKS(200));
//     }
// }

extern "C" void app_main()
{
    esp_log_level_set("*", ESP_LOG_DEBUG);

    main_controls_main();
    // main_test_leds();
    // main_test_buttons();

    // main_print_adc_value();
}