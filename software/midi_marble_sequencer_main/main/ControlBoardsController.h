#pragma once

#include "i2c_config.h"
#include "controls_main_common.h"
#include "Sequencer.h"

#include <driver/i2c_master.h>

#define I2C_MASTER_SCL_GPIO GPIO_NUM_4
#define I2C_MASTER_SDA_GPIO GPIO_NUM_5
#define I2C_MASTER_TIMER_PERIOD 30000 // in us
// #define I2C_MASTER_TIMER_PERIOD 60000 // in us

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

    uint8_t *_write_buffer;
    uint8_t *_read_buffer;
};