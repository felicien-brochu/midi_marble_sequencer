#pragma once

#include "IRSensBoards.h"

#include <esp_adc/adc_oneshot.h>

#define ONESHOT_IR_LED_RISE_MS 0
#define CLEAN_ADC_CYCLES 30

inline void _read_values(IRSensBoards *ir_sens_boards, adc_oneshot_unit_handle_t adc_handle, int *values, int multisampling);
inline void _read_value(IRSensBoards *ir_sens_boards, adc_oneshot_unit_handle_t adc_handle, int *value, int multisampling);
inline void _clean_adc_input(IRSensBoards *ir_sens_boards, adc_oneshot_unit_handle_t adc_handle);
inline void _average_multisampled_values(uint8_t ir_sens_on_board, int *values, int multisampling);

class IRSensReader
{
public:
    IRSensReader(IRSensBoards *ir_sens_boards);

    void read_board_values(int *values_off, int *values_on, uint8_t board_index, int multisampling = 1);

    void read_values_off(int *values_off, int multisampling);

    void read_values_on(int *values_on, int multisampling);

    void read_sensor_value(int *value_off, int *value_on, uint8_t board_index, uint8_t sensor_index, int multisampling);

private:
    IRSensBoards *_ir_sens_boards;

    adc_oneshot_unit_handle_t _adc_handle;
};