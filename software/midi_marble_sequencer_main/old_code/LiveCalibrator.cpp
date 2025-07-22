#include "LiveCalibrator.h"
#include "SensorsCalibrationData.h"
#include "marble_type.h"

#define A_INTERSECT_ORIGIN -625.

LiveCalibrator::LiveCalibrator(IRSensReader board_reader, uint8_t multisampling) : _board_reader(board_reader)
{
    _multisampling = multisampling;
    _calibration_sens_value = 0;
    _calibration_sens_value_ref = marble_types_means[NUM_IR_SENS_BOARDS - 1][NUM_IR_SENS_BY_BOARD - 1][GREEN_MARBLE];
}

uint16_t LiveCalibrator::correct_value(uint16_t value, uint8_t board_index, uint8_t sensor_index)
{
    double a = ((double) (value - A_INTERSECT_ORIGIN)) / (_calibration_sens_value - A_INTERSECT_ORIGIN);
    double corrected_value = a * ((double)_calibration_sens_value_ref - _calibration_sens_value) + value;

    if (corrected_value < 0) {
        corrected_value = 0;
    }

    return (uint16_t) corrected_value;
}

void LiveCalibrator::calibrate()
{
    uint32_t value_off, value_on;
    _board_reader.read_sensor_value(&value_off, &value_on, NUM_IR_SENS_BOARDS - 1, NUM_IR_SENS_BY_BOARD - 1, _multisampling);

    _calibration_sens_value = value_off - value_on;
    printf("\nLiveCalibrator: %ld instead of %d\n\n", _calibration_sens_value, _calibration_sens_value_ref);
}
