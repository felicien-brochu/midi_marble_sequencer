#include "MarbleChangeDetector.h"

MarbleChangeDetector::MarbleChangeDetector() : _board_reader(&_ir_sens_board)
{
    _values_on = (int *)malloc(_ir_sens_board.ir_sens_on_board * sizeof(int));
    _values_off = (int *)malloc(_ir_sens_board.ir_sens_on_board * sizeof(int));

    _marble_types = (marble_type_t *)malloc(_ir_sens_board.ir_sens_on_board * sizeof(marble_type_t));

    for (size_t i = 0; i < _ir_sens_board.ir_sens_on_board; i++)
    {
        _marble_types[i] = NO_MARBLE;
    }
}

marble_type_t _get_detected_marble_type(uint16_t *thresholds, int value)
{
    marble_type_t marble_type = NO_MARBLE;

    for (; marble_type < WHITE_MARBLE; marble_type = (marble_type_t)(marble_type + 1))
    {
        if (value < thresholds[marble_type])
        {
            break;
        }
    }

    return marble_type;
}

void MarbleChangeDetector::update()
{
    _board_reader.read_values(_values_off, _values_on, 4);

    for (int i = 0; i < _ir_sens_board.ir_sens_on_board; i++)
    {
        marble_type_t new_marble_type = _get_detected_marble_type(_marble_types_thresholds[i], _values_off[i] - _values_on[i]);
        if (new_marble_type != _marble_types[i]) {
            _marble_types[i] = new_marble_type;
            _print_sensor_marble_type(i);
        }
    }
}

void MarbleChangeDetector::_print_sensor_marble_type(int sensor_channel){
    printf("Sens %2d: %s\n", sensor_channel, marble_type_to_string(_marble_types[sensor_channel]));
}