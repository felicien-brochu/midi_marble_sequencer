#pragma once

#include "controls_main_common.h"
#include "controls_pages_common.h"

#define I2C_CONFIG_PORT 0
#define I2C_CONFIG_CLOCK_SPEED_HZ 100000
#define I2C_CONFIG_TRANSACTION_DELAY_US 30000
#define I2C_SEND_BUF_DEPTH 256
#define I2C_RECEIVE_BUF_DEPTH 256


#define I2C_CONFIG_CONTROLS_PAGES_ADDR 0x04
#define I2C_CONFIG_CONTROLS_MAIN_ADDR 0x02

#define I2C_CONFIG_REQ_TIME_US(REQ_TYPE) (uint16_t) ( ( (float) (sizeof(REQ_TYPE) + 1 ) * 9. ) / ( (float) I2C_CONFIG_CLOCK_SPEED_HZ / 1000000. ) ) //< time in us to send a request of the specified REQ_TYPE on the i2c bus

#define I2C_CONFIG_MIN_TRANSACTION_TIME_US ( ( I2C_CONFIG_REQ_TIME_US(controls_main_value_t) + I2C_CONFIG_REQ_TIME_US(controls_main_display_t) + I2C_CONFIG_REQ_TIME_US(controls_pages_value_t) + I2C_CONFIG_REQ_TIME_US(controls_pages_display_t) ) + I2C_CONFIG_TRANSACTION_DELAY_US )

#define I2C_CONFIG_SLAVE_FIFO_DELAY_US 20000


typedef enum {
    I2C_EVENT_TYPE_NONE = -1,
    I2C_EVENT_TYPE_RX = 0,
    I2C_EVENT_TYPE_TX,
} i2c_event_type_t;