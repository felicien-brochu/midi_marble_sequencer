#include "BPMPotentiometer.h"
#include "I2CSlaveController.h"
#include "PushButtonsController.h"
#include "InteractionController.h"
#include "DisplayController.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <Arduino.h>

void i2c_slave_controller_task(void *i2c_slave_controller_arg)
{
    I2CSlaveController *i2c_slave_controller = (I2CSlaveController *)i2c_slave_controller_arg;
    i2c_slave_controller->main_task();
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
    // vTaskDelay(pdMS_TO_TICKS(I2C_CONFIG_TRANSACTION_DELAY_US / 1000));
}
