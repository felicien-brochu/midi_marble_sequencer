#pragma once

#include <driver/gpio.h>
#include <hal/adc_types.h>
#include <CD74HC4067.h>

#define NUM_IR_SENS_BOARDS 16

#define IR_SENS_BOARD_MUX_S0_PIN GPIO_NUM_39
#define IR_SENS_BOARD_MUX_S1_PIN GPIO_NUM_40
#define IR_SENS_BOARD_MUX_S2_PIN GPIO_NUM_41
#define IR_SENS_BOARD_MUX_S3_PIN GPIO_NUM_42

// ESP-32-S3 GPIO 4
#define IR_SENS_ADC_UNIT ADC_UNIT_1
#define IR_SENS_ADC_CHANNEL ADC_CHANNEL_5

#define IR_LED_POWER_PIN GPIO_NUM_7


#define NUM_IR_SENS_BY_BOARD 16

#define IR_SENS_MUX_S0_PIN GPIO_NUM_35
#define IR_SENS_MUX_S1_PIN GPIO_NUM_36
#define IR_SENS_MUX_S2_PIN GPIO_NUM_37
#define IR_SENS_MUX_S3_PIN GPIO_NUM_38

class IRSensBoards
{
public:
    IRSensBoards();

    void disable_leds();

    void enable_leds();

    void select_board(uint8_t board_index);
    void select_sensor(uint8_t sensor_index);

    static const uint8_t ir_sens_on_board;
    static const adc_unit_t adc_unit = IR_SENS_ADC_UNIT;
    static const adc_channel_t adc_channel = IR_SENS_ADC_CHANNEL;

private:
    CD74HC4067 _ir_sens_board_mux;
    CD74HC4067 _ir_sens_mux;

    uint8_t _selected_board;
    uint8_t _selected_sensor;

    void _init_leds();
};