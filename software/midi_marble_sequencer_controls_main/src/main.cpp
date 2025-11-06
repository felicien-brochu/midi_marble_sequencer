#include "BPMPotentiometer.h"
#include "I2CSlaveController.h"
#include "InteractionController.h"
#include "DisplayController.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Arduino.h>

void i2c_slave_controller_task(void *i2c_slave_controller_arg)
{
    I2CSlaveController *i2c_slave_controller = (I2CSlaveController *)i2c_slave_controller_arg;
    i2c_slave_controller->main_task();
}

void main_test_leds()
{
    LEDColumn led_column;
    PlayPauseBPMLED play_pause_led;

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

DisplayController *display_controller = new DisplayController();
InteractionController *interaction_controller = new InteractionController();
I2CSlaveController *i2c_slave_controller = new I2CSlaveController(interaction_controller, display_controller);

void setup()
{
    TaskHandle_t i2c_task_handle;
    xTaskCreate(i2c_slave_controller_task, "I2CSlaveController", 10000, i2c_slave_controller, 1, &i2c_task_handle);

    interaction_controller->set_event_listener(i2c_slave_controller);
    interaction_controller->update();
    i2c_slave_controller->write_to_buffer();
}

void loop()
{
    interaction_controller->update();
}
