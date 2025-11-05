#pragma once

#include <Arduino.h>

// #define BPM_POT_ADC_UNIT ADC_UNIT_1
// #define BPM_POT_ADC_CHANNEL ADC_CHANNEL_6
#define BPM_POT_ADC_GPIO 34
#define BPM_POT_MULTISAMPLING 4
#define BPM_POT_ADC_VALUE_MIN 350
#define BPM_POT_ADC_VALUE_MAX 3450


class BPMPotentiometer
{
public:
    BPMPotentiometer();

    void update();
    float get_normalized_value();
    
private:
    uint16_t _adc_value;
    uint16_t _adc_raw;
    
    uint16_t _read_value_from_adc();
};