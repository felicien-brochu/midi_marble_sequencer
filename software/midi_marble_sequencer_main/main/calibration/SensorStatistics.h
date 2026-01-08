#pragma once

#include "marble_type.h"
#include <cstdint>

class ColorStatistics
{
public:
    ColorStatistics();

    void push_sample(int value_off, int value_on);

    int32_t min;
    int32_t max;
    int32_t sum;
    uint16_t nb_samples;
    int32_t mean;
};

class SensorStatistics
{
public:
    SensorStatistics();

    void push_sample(marble_type_t color, int value_off, int value_on);
    void compute_thresholds(uint16_t *thresholds);

    ColorStatistics color_statistics[NUM_MARBLE_TYPE];
};
