#pragma once

#include "IRSensBoards.h"

#include <esp_adc/adc_oneshot.h>

#define ONESHOT_IR_LED_RISE_MS 0
#define CLEAN_ADC_CYCLES 30
#define IR_SENSOR_MULTISAMPLING 4

#define NUM_VALUE_BY_COLUMN (NUM_IR_SENS_BY_BOARD / 2)

inline void _read_values(IRSensBoards *ir_sens_boards, adc_oneshot_unit_handle_t adc_handle, uint32_t *values, int multisampling);
inline void _read_value(IRSensBoards *ir_sens_boards, adc_oneshot_unit_handle_t adc_handle, uint32_t *value, int multisampling);
inline void _clean_adc_input(IRSensBoards *ir_sens_boards, adc_oneshot_unit_handle_t adc_handle);
inline void _average_multisampled_values(uint8_t ir_sens_on_board, uint32_t *values, int multisampling);

class IRSensReader
{
public:
    IRSensReader(IRSensBoards *ir_sens_boards);

    void read_column(uint32_t *_values_off, uint32_t *_values_on, uint8_t column_index);

    void read_sub_board_values(uint32_t *values_off, uint32_t *values_on, uint8_t board_index, const uint8_t *sensor_list, size_t sensor_list_size, int multisampling);

    void read_board_values(uint32_t *values_off, uint32_t *values_on, uint8_t board_index, int multisampling = 1);

    void read_values_off(uint32_t *values_off, int multisampling);

    void read_sub_board_values_on(uint32_t *values_on, const uint8_t *sensor_list, size_t sensor_list_size, int multisampling);

    void read_sub_board_values_off(uint32_t *values_off, const uint8_t *sensor_list, size_t sensor_list_size, int multisampling);

    void read_values_on(uint32_t *values_on, int multisampling);

    void read_sensor_value(uint32_t *value_off, uint32_t *value_on, uint8_t board_index, uint8_t sensor_index, int multisampling);

private:
    IRSensBoards *_ir_sens_boards;

    adc_oneshot_unit_handle_t _adc_handle;
};