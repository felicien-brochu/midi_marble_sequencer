#include "IRSensBoardReaderOneShot.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_timer.h>

static const char *TAG = "IR_SENS_BOARD_READER_ONE_SHOT";

static int _adc_raw[1];

IRSensBoardReaderOneShot::IRSensBoardReaderOneShot(IRSensBoard *ir_sens_board)
{
    _ir_sens_board = ir_sens_board;

    adc_oneshot_chan_cfg_t adc_channel_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    adc_oneshot_unit_init_cfg_t adc_init_config = {
        .unit_id = _ir_sens_board->adc_unit,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_init_config, &_adc_handle));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(_adc_handle, _ir_sens_board->adc_channel, &adc_channel_config));
}

void IRSensBoardReaderOneShot::read_values(int *values_off, int *values_on, int multisampling)
{
    uint64_t t0 = esp_timer_get_time();
    read_values_off(values_off, multisampling);
    read_values_on(values_on, multisampling);
    ESP_LOGD(TAG, "Read time: %llu us", esp_timer_get_time() - t0);
}

void IRSensBoardReaderOneShot::read_values_on(int *values_on, int multisampling)
{
    _ir_sens_board->enable_leds();
    #if (ONESHOT_IR_LED_RISE_MS > 0)
        vTaskDelay(pdMS_TO_TICKS(ONESHOT_IR_LED_RISE_MS));
        _clean_adc_input(_ir_sens_board, _adc_handle);
    #endif

    _read_values(_ir_sens_board, _adc_handle, values_on, multisampling);

    _ir_sens_board->disable_leds();

    _average_multisampled_values(_ir_sens_board->ir_sens_on_board, values_on, multisampling);
}

void IRSensBoardReaderOneShot::read_values_off(int *values_off, int multisampling)
{
    _clean_adc_input(_ir_sens_board, _adc_handle);
    _read_values(_ir_sens_board, _adc_handle, values_off, multisampling);
    _average_multisampled_values(_ir_sens_board->ir_sens_on_board, values_off, multisampling);
}

inline void _read_values(IRSensBoard *ir_sens_board, adc_oneshot_unit_handle_t adc_handle, int *values, int multisampling)
{
    // for (int i = 0; i < multisampling; i++)
    // {
    //     for (int j = ir_sens_board->ir_sens_on_board - 1; j >= 0; j--)
    //     {
    //         ir_sens_board->select_sensor(j);
    //         if (i == 0)
    //         {
    //             values[j] = 0;
    //         }
    //         ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ir_sens_board->adc_channel, _adc_raw));
    //         values[j] += _adc_raw[0];
    //     }
    // }
    for (int i = 0; i < ir_sens_board->ir_sens_on_board; i++)
    {
        ir_sens_board->select_sensor(i);
        for (int j = 0; j < multisampling; j++)
        {
            if (j == 0)
            {
                values[i] = 0;
            }
            ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ir_sens_board->adc_channel, _adc_raw));
            values[i] += _adc_raw[0];
        }
    }
}

inline void _average_multisampled_values(uint8_t ir_sens_on_board, int *values, int multisampling)
{
    for (int i = 0; i < ir_sens_on_board; i++)
    {
        values[i] = values[i] / multisampling;
    }
}

inline void _clean_adc_input(IRSensBoard *ir_sens_board, adc_oneshot_unit_handle_t adc_handle)
{
    for (int i = 0; i < CLEAN_ADC_CYCLES; i++)
    {
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ir_sens_board->adc_channel, _adc_raw));
    }
}