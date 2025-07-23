#pragma once

#include <esp_adc/adc_oneshot.h>

#define BPM_POT_ADC_UNIT ADC_UNIT_1
#define BPM_POT_ADC_CHANNEL ADC_CHANNEL_6
#define BPM_POT_MULTISAMPLING 4

// Min value: 350
// Max value: 3450


class BPMPotentiometer
{
public:
    BPMPotentiometer(adc_oneshot_unit_handle_t &adc_handle);

    void update();
    
private:
    uint16_t _value;
    
    adc_oneshot_unit_handle_t &_adc_handle;
    int _adc_raw[1];
    
    uint16_t _read_value_from_adc();
};