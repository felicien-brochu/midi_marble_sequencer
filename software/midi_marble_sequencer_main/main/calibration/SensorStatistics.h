#pragma once

#include "marble_type.h"
#include <cstdint>

class ColorStatistics
{
public:
    ColorStatistics();

    void push_sample(int value_off, int value_on);

    uint16_t min;
    uint16_t max;
    uint32_t sum;
    uint16_t nb_samples;
    uint16_t mean;
};

class SensorStatistics
{
public:
    SensorStatistics();

    void push_sample(marble_type_t color, int value_off, int value_on);
    void compute_thresholds(uint16_t *thresholds);

    ColorStatistics color_statistics[NUM_MARBLE_TYPE];
};
