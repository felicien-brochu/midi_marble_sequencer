#include "BPMPotentiometer.h"


BPMPotentiometer::BPMPotentiometer(adc_oneshot_unit_handle_t &adc_handle) : _adc_handle(adc_handle)
{
    adc_oneshot_chan_cfg_t adc_channel_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(_adc_handle, BPM_POT_ADC_CHANNEL, &adc_channel_config));
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
        ESP_ERROR_CHECK(adc_oneshot_read(_adc_handle, BPM_POT_ADC_CHANNEL, _adc_raw));
        adc_raw_sum += _adc_raw[0];
    }

    int adc_mean_value = adc_raw_sum / BPM_POT_MULTISAMPLING;
    return (uint16_t) adc_mean_value;
}