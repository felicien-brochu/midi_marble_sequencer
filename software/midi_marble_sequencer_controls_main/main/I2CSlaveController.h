#pragma once

#include "i2c_config.h"
#include "InteractionController.h"
#include "DisplayController.h"
#include "controls_main_common.h"

#include <driver/i2c_slave.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#define I2C_SLAVE_SCL_GPIO GPIO_NUM_22
#define I2C_SLAVE_SDA_GPIO GPIO_NUM_21


class I2CSlaveController
{
public:
    I2CSlaveController(InteractionController &interaction_controller, DisplayController &display_controller);

    void main_task();

private:
    InteractionController _interaction_controller;
    DisplayController _display_controller;

    i2c_slave_dev_handle_t _slave_handle;
    QueueHandle_t _receive_queue;

    controls_main_display_t _controls_main_display;
    controls_main_value_t _controls_main_value;
    uint8_t *_write_buffer;
    uint8_t *_read_buffer;
};