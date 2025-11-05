#pragma once

#include "i2c_config.h"
#include "InteractionController.h"
#include "DisplayController.h"
#include "controls_pages_common.h"

#include <driver/i2c_slave.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#define I2C_SLAVE_SCL_GPIO GPIO_NUM_22
#define I2C_SLAVE_SDA_GPIO GPIO_NUM_21


class I2CSlaveController
{
public:
    I2CSlaveController(InteractionController *interaction_controller, DisplayController *display_controller);

    void main_task();

private:
    InteractionController *_interaction_controller;
    DisplayController *_display_controller;

    i2c_slave_dev_handle_t _slave_handle;
    QueueHandle_t _event_queue;


    void _on_receive(const i2c_slave_rx_done_event_data_t *event_data);
    void _on_request();
};