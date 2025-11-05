#pragma once

#include "i2c_config.h"
#include "controls_main_common.h"
#include "controls_pages_common.h"
#include "Sequencer.h"

#include <driver/i2c_master.h>

#define I2C_MASTER_SCL_GPIO GPIO_NUM_4
#define I2C_MASTER_SDA_GPIO GPIO_NUM_5

class ControlBoardsController
{
public:
    ControlBoardsController(Sequencer &sequencer);

    void start_control_boards_task();
    void main_task();

private:
    Sequencer &_sequencer;

    i2c_master_bus_handle_t _i2c_bus_handle;
    i2c_master_dev_handle_t _controls_main_device_handle;
    i2c_master_dev_handle_t _controls_pages_device_handle;

    uint8_t *_main_write_buffer;
    uint8_t *_pages_write_buffer;
    controls_main_value_t _last_controls_main_value;

    void _init_main_controls_i2c();
    void _init_pages_controls_i2c();

    void _talk_to_controls_main();
    esp_err_t _read_controls_main_events();
    esp_err_t _transmit_controls_main_display();
    void _talk_to_controls_pages();
    esp_err_t _read_controls_pages_events();
    esp_err_t _transmit_controls_pages_display();
};