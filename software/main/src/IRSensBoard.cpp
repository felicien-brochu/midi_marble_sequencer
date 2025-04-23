#include "IRSensBoard.h"

const uint8_t IRSensBoard::ir_sens_channels[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
const uint8_t IRSensBoard::ir_sens_on_board = 16;

IRSensBoard::IRSensBoard() : _ir_sens_mux(IR_SENS_BOARD_MUX_S0_PIN, IR_SENS_BOARD_MUX_S1_PIN, IR_SENS_BOARD_MUX_S2_PIN, IR_SENS_BOARD_MUX_S3_PIN)
{
    _init_leds();
}

void IRSensBoard::_init_leds()
{
    gpio_reset_pin(IR_LED_POWER_PIN);
    gpio_set_direction(IR_LED_POWER_PIN, GPIO_MODE_OUTPUT);
    disable_leds();
}

void IRSensBoard::disable_leds()
{
    gpio_set_level(IR_LED_POWER_PIN, 1);
}

void IRSensBoard::enable_leds()
{
    gpio_set_level(IR_LED_POWER_PIN, 0);
}

void IRSensBoard::select_sensor(uint8_t sensor_index)
{
    if (sensor_index >= IRSensBoard::ir_sens_on_board) {
        return;
    }

    uint8_t channel = IRSensBoard::ir_sens_channels[sensor_index];
    _ir_sens_mux.channel(channel);
}

uint8_t IRSensBoard::get_sensor_channel(uint8_t sensor_index)
{
    if (sensor_index >= IRSensBoard::ir_sens_on_board)
    {
        return -1;
    }

    return IRSensBoard::ir_sens_channels[sensor_index];
}