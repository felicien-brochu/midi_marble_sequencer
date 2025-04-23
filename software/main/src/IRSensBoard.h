#pragma once

#include <driver/gpio.h>
#include <hal/adc_types.h>
#include <CD74HC4067.h>

#define MAX_IR_SENS_ON_BOARD 16

#define IR_LED_POWER_PIN GPIO_NUM_5

// S3
#define IR_SENS_BOARD_MUX_S0_PIN GPIO_NUM_35
#define IR_SENS_BOARD_MUX_S1_PIN GPIO_NUM_36
#define IR_SENS_BOARD_MUX_S2_PIN GPIO_NUM_37
#define IR_SENS_BOARD_MUX_S3_PIN GPIO_NUM_38

// ESP32-WROOM-32D
// #define IR_SENS_BOARD_MUX_S0_PIN GPIO_NUM_33
// #define IR_SENS_BOARD_MUX_S1_PIN GPIO_NUM_25
// #define IR_SENS_BOARD_MUX_S2_PIN GPIO_NUM_26
// #define IR_SENS_BOARD_MUX_S3_PIN GPIO_NUM_27

// ESP-32-S3 GPIO 4
#define IR_SENS_ADC_UNIT ADC_UNIT_1
#define IR_SENS_ADC_CHANNEL ADC_CHANNEL_3

// ESP32-WROOM-32D GPIO 32
// #define IR_SENS_ADC_UNIT ADC_UNIT_1
// #define IR_SENS_ADC_CHANNEL ADC_CHANNEL_4

class IRSensBoard
{
public:
    IRSensBoard();


    void disable_leds();

    void enable_leds();

    void select_sensor(uint8_t channel);

    uint8_t get_sensor_channel(uint8_t sensor_index);

    static const uint8_t ir_sens_on_board;
    static const adc_unit_t adc_unit = IR_SENS_ADC_UNIT;
    static const adc_channel_t adc_channel = IR_SENS_ADC_CHANNEL;

private:
    static const uint8_t ir_sens_channels[MAX_IR_SENS_ON_BOARD];
    CD74HC4067 _ir_sens_mux;

    void _init_leds();
};