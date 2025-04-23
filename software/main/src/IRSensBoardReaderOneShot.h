#pragma once

#include "IRSensBoardReader.h"

#include <esp_adc/adc_oneshot.h>

#define ONESHOT_IR_LED_RISE_MS 0
#define CLEAN_ADC_CYCLES 30

inline void _read_values(IRSensBoard *ir_sens_board, adc_oneshot_unit_handle_t adc_handle, int *values, int multisampling);
inline void _clean_adc_input(IRSensBoard *ir_sens_board, adc_oneshot_unit_handle_t adc_handle);
inline void _average_multisampled_values(uint8_t ir_sens_on_board, int *values, int multisampling);

class IRSensBoardReaderOneShot : public IRSensBoardReader
{
public:
    IRSensBoardReaderOneShot(IRSensBoard *ir_sens_board);

    void read_values(int *values_off, int *values_on, int multisampling = 1);

    void read_values_off(int *values_off, int multisampling);

    void read_values_on(int *values_on, int multisampling);

private:
    IRSensBoard *_ir_sens_board;

    adc_oneshot_unit_handle_t _adc_handle;
};