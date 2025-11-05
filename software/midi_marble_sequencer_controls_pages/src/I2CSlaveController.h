#pragma once

#include "i2c_config.h"
#include "InteractionController.h"
#include "DisplayController.h"
#include "IEventListener.h"
#include "controls_pages_common.h"

#include <esp32-hal-timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#define I2C_SLAVE_SCL_GPIO GPIO_NUM_22
#define I2C_SLAVE_SDA_GPIO GPIO_NUM_21


class I2CSlaveController : public IEventListener
{
public:
    I2CSlaveController(InteractionController *interaction_controller, DisplayController *display_controller);

    void main_task();
    void on_receive(int len);
    void on_request();
    void on_event(controls_pages_value_t controls_pages_value);
    void write_to_buffer();
    
    static I2CSlaveController *instance;
    xSemaphoreHandle timer_semaphore;
    
private:
    InteractionController *_interaction_controller;
    DisplayController *_display_controller;
    
    uint8_t *_read_buffer;

    uint16_t _requests_since_last_read;
    uint8_t *_write_buffer;
    hw_timer_t *_write_timer;
    xSemaphoreHandle _write_buffer_lock;
    
    void _schedule_next_write();
};