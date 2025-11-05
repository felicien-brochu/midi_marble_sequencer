#include "BPMPotentiometer.h"
#include <Arduino.h>


BPMPotentiometer::BPMPotentiometer()
{
    // analogSetAttenuation(ADC_ATTENDB_MAX);
    // analogSetWidth(12);
}

void BPMPotentiometer::update()
{
    uint16_t adc_value = _read_value_from_adc();
    _adc_value = adc_value;
}

float BPMPotentiometer::get_normalized_value()
{
    float normalized_value = ((float) _adc_value - BPM_POT_ADC_VALUE_MIN) / (BPM_POT_ADC_VALUE_MAX - BPM_POT_ADC_VALUE_MIN);

    return normalized_value;
}

uint16_t BPMPotentiometer::_read_value_from_adc()
{
    int adc_raw_sum = 0;
    for (size_t i = 0; i < BPM_POT_MULTISAMPLING; i++)
    {
        _adc_raw = analogRead(BPM_POT_ADC_GPIO);
        adc_raw_sum += _adc_raw;
    }

    int adc_mean_value = adc_raw_sum / BPM_POT_MULTISAMPLING;
    return (uint16_t) adc_mean_value;
}