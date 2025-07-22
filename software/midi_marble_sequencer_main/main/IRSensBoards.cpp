#include "IRSensBoards.h"

IRSensBoards::IRSensBoards() : _ir_sens_board_mux(IR_SENS_BOARD_MUX_S0_PIN, IR_SENS_BOARD_MUX_S1_PIN, IR_SENS_BOARD_MUX_S2_PIN, IR_SENS_BOARD_MUX_S3_PIN), _ir_sens_mux(IR_SENS_MUX_S0_PIN, IR_SENS_MUX_S1_PIN, IR_SENS_MUX_S2_PIN, IR_SENS_MUX_S3_PIN)
{
    select_board(0);
    select_sensor(0);
    _init_leds();
}

void IRSensBoards::_init_leds()
{
    gpio_reset_pin(IR_LED_POWER_PIN);
    gpio_set_direction(IR_LED_POWER_PIN, GPIO_MODE_OUTPUT);
    disable_leds();
}

void IRSensBoards::disable_leds()
{
    gpio_set_level(IR_LED_POWER_PIN, 1);
}

void IRSensBoards::enable_leds()
{
    gpio_set_level(IR_LED_POWER_PIN, 0);
}

void IRSensBoards::select_board(uint8_t board_index)
{
    if (board_index >= NUM_IR_SENS_BOARDS)
    {
        return;
    }
    _selected_board = board_index;
    _ir_sens_board_mux.channel(board_index);
}

void IRSensBoards::select_sensor(uint8_t sensor_index)
{
    if (sensor_index >= NUM_IR_SENS_BY_BOARD) {
        return;
    }
    _selected_sensor = sensor_index;
    _ir_sens_mux.channel(sensor_index);
}