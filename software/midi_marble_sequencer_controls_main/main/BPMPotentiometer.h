#pragma once

#include <esp_adc/adc_oneshot.h>

#define BPM_POT_ADC_UNIT ADC_UNIT_1
#define BPM_POT_ADC_CHANNEL ADC_CHANNEL_6
#define BPM_POT_MULTISAMPLING 4
#define BPM_POT_ADC_VALUE_MIN 350
#define BPM_POT_ADC_VALUE_MAX 3450


class BPMPotentiometer
{
public:
    BPMPotentiometer(adc_oneshot_unit_handle_t &adc_handle);

    void update();
    float get_normalized_value();
    
private:
    uint16_t _adc_value;
    
    adc_oneshot_unit_handle_t &_adc_handle;
    int _adc_raw[1];
    
    uint16_t _read_value_from_adc();
};