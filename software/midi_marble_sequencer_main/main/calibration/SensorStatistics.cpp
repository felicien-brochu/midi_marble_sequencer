#include "SensorStatistics.h"
#include <cstddef>


ColorStatistics::ColorStatistics()
{
    min = 1 << 15;
    max = 0;
    sum = 0;
    nb_samples = 0;
    mean = 0;
}

void ColorStatistics::push_sample(int value_off, int value_on)
{
    int diff = value_off - value_on;

    if (min > diff)
    {
        min = diff;
    }

    if (max < diff)
    {
        max = diff;
    }

    sum += diff;

    nb_samples++;
    mean = sum / nb_samples;
}


SensorStatistics::SensorStatistics()
{
}

void SensorStatistics::push_sample(marble_type_t color, int value_off, int value_on)
{
    color_statistics[color].push_sample(value_off, value_on);
}

void SensorStatistics::compute_thresholds(uint16_t *thresholds)
{
    for (size_t i = 0; i < NUM_MARBLE_TYPE - 1; i++)
    {
        // uint16_t up_var = color_statistics[i].max - color_statistics[i].mean;
        // uint16_t down_var = color_statistics[i + 1].mean - color_statistics[i + 1].min;

        // float k = ((float) up_var) / (up_var + down_var);
        // uint16_t threshold = (uint16_t) (color_statistics[i].max + k * (color_statistics[i + 1].min - color_statistics[i].max));

        // // First and last threshold are not scaled by variance because first and last marble types have truncated (saturated) measurements
        // // Instead we choose the middle between low max et high min.
        // if (i == 0 || i == NUM_MARBLE_TYPE - 2)
        // {
        uint16_t threshold = color_statistics[i].mean + ((color_statistics[i + 1].mean - color_statistics[i].mean) / 2);
        // }
        thresholds[i] = threshold;
    }
}