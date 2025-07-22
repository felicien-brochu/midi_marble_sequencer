#pragma once
#include "IRSensReader.h"

class LiveCalibrator
{
public:
    LiveCalibrator(IRSensReader board_reader, uint8_t multisampling);

    uint16_t correct_value(uint16_t value, uint8_t board_index, uint8_t sensor_index);
    void calibrate();

private:
    IRSensReader _board_reader;
    uint8_t _multisampling;
    uint16_t _calibration_sens_value_ref;

    uint32_t _calibration_sens_value;
};