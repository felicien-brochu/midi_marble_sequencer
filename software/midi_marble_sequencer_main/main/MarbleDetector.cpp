#include "MarbleDetector.h"
#include "SensorsCalibrationData.h"

MarbleDetector::MarbleDetector() : _ir_sens_boards(), _ir_sens_reader(&_ir_sens_boards)
{
    _mutex = xSemaphoreCreateMutex();
}

void MarbleDetector::detect_eighth_note_marbles(uint8_t eighth_note_index, marble_type_t *eighth_note_marble_types)
{
    if (xSemaphoreTake(_mutex, portMAX_DELAY))
    {
        _ir_sens_reader.read_column(_values_off, _values_on, eighth_note_index);

        _convert_eighth_note_values_to_marble_types(eighth_note_index, eighth_note_marble_types);
        xSemaphoreGive(_mutex);
    }
}

void MarbleDetector::detect_measure_marbles(uint8_t measure_index, marble_type_t *measure_marble_types)
{
    if (xSemaphoreTake(_mutex, portMAX_DELAY))
    {
        for (size_t i = 0; i < SEQUENCER_EIGHTH_NOTE_BY_MEASURE_NUM; i++)
        {
            uint8_t eighth_note_index = measure_index * SEQUENCER_EIGHTH_NOTE_BY_MEASURE_NUM + i;
            _ir_sens_reader.read_column(_values_off, _values_on, eighth_note_index);
            _convert_eighth_note_values_to_marble_types(eighth_note_index, &measure_marble_types[i * SEQUENCER_TRACKS_NUM]);
        }

        xSemaphoreGive(_mutex);
    }
}

void MarbleDetector::_convert_eighth_note_values_to_marble_types(uint8_t eighth_note_index, marble_type_t *eighth_note_marble_types)
{
    for (size_t i = 0; i < SEQUENCER_TRACKS_NUM; i++)
    {
        marble_type_t marble_type = _get_marble_type(i, _get_thresholds_for_eighth_note_value(eighth_note_index, i));
        eighth_note_marble_types[i] = marble_type;
    }
}

const uint16_t *MarbleDetector::_get_thresholds_for_eighth_note_value(uint8_t eighth_note_index, uint8_t value_index)
{
    uint8_t board_index = eighth_note_index / 2;
    uint8_t sensor_index = eighth_note_index % 2 == 0 ? value_index : NUM_IR_SENS_BY_BOARD - 1 - value_index;

    return marble_types_thresholds[board_index][sensor_index];
}

marble_type_t MarbleDetector::_get_marble_type(uint8_t value_index, const uint16_t *thresholds)
{      
    marble_type_t marble_type = NO_MARBLE;
    int value = _values_off[value_index] - _values_on[value_index];
    
    for (; marble_type < NUM_MARBLE_TYPE - 1; marble_type = (marble_type_t)(marble_type + 1))
    {
        if (value < thresholds[marble_type])
        {
            break;
        }
    }

    return marble_type;
}