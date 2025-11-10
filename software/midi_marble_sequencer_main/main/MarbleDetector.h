#pragma once

#include "marble_type.h"
#include "IRSensBoards.h"
#include "IRSensReader.h"
#include "sequencer_config.h"
#include "calibration/CalibrationStorage.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class MarbleDetector
{
public:
    MarbleDetector();

    void detect_eighth_note_marbles(uint8_t eighth_note_index, marble_type_t *eighth_note_marble_types);
    void detect_measure_marbles(uint8_t measure_index, marble_type_t *measure_marble_types);


private:
    IRSensBoards _ir_sens_boards;
    IRSensReader _ir_sens_reader;
    
    CalibrationStorage _calibration_storage;
    uint16_t (*_marble_types_thresholds)[NUM_IR_SENS_BY_BOARD][NUM_MARBLE_TYPE - 1];
    SemaphoreHandle_t _mutex;

    uint32_t _values_on[SEQUENCER_TRACKS_NUM];
    uint32_t _values_off[SEQUENCER_TRACKS_NUM];
    
    
    void _convert_eighth_note_values_to_marble_types(uint8_t eighth_note_index, marble_type_t *eighth_note_marble_types);
    const uint16_t *_get_thresholds_for_eighth_note_value(uint8_t eighth_note_index, uint8_t value_index);
    marble_type_t _get_marble_type(uint8_t value_index, const uint16_t *thresholds);
    void _print_marble_types_thresholds();
};