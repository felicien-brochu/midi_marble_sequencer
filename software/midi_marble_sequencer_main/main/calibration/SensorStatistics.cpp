#include "SensorStatistics.h"
#include <cstddef>
#include <esp_log.h>

static const char *TAG = "SensorStatistics";


ColorStatistics::ColorStatistics()
{
    min = INT32_MAX;
    max = INT32_MIN;
    sum = 0;
    nb_samples = 0;
    mean = 0;
}

void ColorStatistics::push_sample(int value_off, int value_on)
{
    int32_t diff = value_off - value_on;

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
        int32_t mean_low = color_statistics[i].mean;
        int32_t mean_high = color_statistics[i + 1].mean;
        
        // Correct for sensor saturation at boundaries
        // Theoretical interval size = 55 + mean * 0.05
        
        // Check if NO_MARBLE (first color) is saturating down
        if (i == 0)
        {
            int32_t measured_interval = color_statistics[i].max - color_statistics[i].min;
            // Theoretical interval derived from: interval = 55 + mean * 0.05
            // where mean = max - interval/2, solving gives:
            float theoretical_interval = (55.0f + color_statistics[i].max * 0.05f) / 1.025f;
            
            if (measured_interval < theoretical_interval)
            {
                // Saturating down, correct the mean
                mean_low = color_statistics[i].max - (int32_t)(theoretical_interval / 2.0f);
                ESP_LOGW(TAG, "NO_MARBLE saturating down. Corrected mean from %ld to %ld", 
                         color_statistics[i].mean, mean_low);
            }
        }
        
        // Check if last color is saturating up
        if (i + 1 == NUM_MARBLE_TYPE - 1)
        {
            int32_t measured_interval = color_statistics[i + 1].max - color_statistics[i + 1].min;
            // Theoretical interval derived from: interval = 55 + mean * 0.05
            // where mean = min + interval/2, solving gives:
            float theoretical_interval = (55.0f + color_statistics[i + 1].min * 0.05f) / 0.975f;
            
            if (measured_interval < theoretical_interval)
            {
                // Saturating up, correct the mean
                mean_high = color_statistics[i + 1].min + (int32_t)(theoretical_interval / 2.0f);
                ESP_LOGW(TAG, "WHITE saturating up. Corrected mean from %ld to %ld", 
                         color_statistics[i + 1].mean, mean_high);
            }
        }
        
        int32_t threshold;
        
        // Protect against inverted means
        if (mean_high > mean_low)
        {
            // Calculate threshold as midpoint between means
            threshold = mean_low + ((mean_high - mean_low) / 2);
        }
        else
        {
            // If means are inverted, use the average (though this indicates a calibration problem)
            threshold = (mean_low + mean_high) / 2;
            ESP_LOGE(TAG, "Inverted means detected for marble types %d and %d during threshold computation.", i, i + 1);
        }
        
        // Verify threshold is between max of lower color and min of higher color
        // This is important after saturation correction at boundaries
        int32_t lower_max = color_statistics[i].max;
        int32_t upper_min = color_statistics[i + 1].min;
        
        if (threshold < lower_max || threshold > upper_min)
        {
            // Threshold is outside the valid range, use midpoint
            threshold = lower_max + ((upper_min - lower_max) / 2);
            ESP_LOGW(TAG, "Threshold for marble types %d and %d outside valid range. Using midpoint between max and min.", i, i + 1);
        }
        
        // Clamp negative thresholds to 0
        if (threshold < 0)
        {
            threshold = 0;
        }
        
        thresholds[i] = threshold;
    }
}