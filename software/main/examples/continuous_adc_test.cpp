/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_adc/adc_continuous.h"
#include "CD74HC4067.h"
#include "driver/gpio.h"

#define MULTISAMPLING_DEPTH 10

#define IR_LED_RISE_MS 0
#define IR_LED_POWER_PIN GPIO_NUM_14
// ESP-32-S3 GPIO 4 (found in pinout)
#define IR_SENS_ADC_UNIT ADC_UNIT_1
#define IR_SENS_ADC_CHANNEL ADC_CHANNEL_3

// ESP32-WROOM-32D GPIO 32
// #define IR_SENS_ADC_UNIT ADC_UNIT_1
// #define IR_SENS_ADC_CHANNEL ADC_CHANNEL_4

#define ADC_READ_LEN       64

// S3
#define MUX_S0_PIN GPIO_NUM_35
#define MUX_S1_PIN GPIO_NUM_36
#define MUX_S2_PIN GPIO_NUM_37
#define MUX_S3_PIN GPIO_NUM_38

// ESP32-WROOM-32D
// #define MUX_S0_PIN GPIO_NUM_33
// #define MUX_S1_PIN GPIO_NUM_25
// #define MUX_S2_PIN GPIO_NUM_26
// #define MUX_S3_PIN GPIO_NUM_27

#define NUM_IR_SENS_ON_BOARD 16

struct ir_sens_value
{
    double value_on;
    double value_off;
    double value_diff;
};

CD74HC4067 ir_sens_mux(MUX_S0_PIN, MUX_S1_PIN, MUX_S2_PIN, MUX_S3_PIN);

static const int ir_sens_addresses[NUM_IR_SENS_ON_BOARD] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

adc_continuous_handle_t adc_handle = NULL;

static adc_channel_t channel[1] = {IR_SENS_ADC_CHANNEL};

static TaskHandle_t s_task_handle;
static const char *TAG = "EXAMPLE";

static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
{
    BaseType_t mustYield = pdFALSE;
    // Notify that ADC continuous driver has done enough number of conversions
    vTaskNotifyGiveFromISR(s_task_handle, &mustYield);

    return (mustYield == pdTRUE);
}

static void continuous_adc_init(adc_channel_t *channel, uint8_t channel_num, adc_continuous_handle_t *out_handle)
{
    adc_continuous_handle_t handle = NULL;

    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 1024,
        .conv_frame_size = ADC_READ_LEN,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 20 * 1000,
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
    };

    adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
    dig_cfg.pattern_num = channel_num;
    for (int i = 0; i < channel_num; i++)
    {
        adc_pattern[i].atten = ADC_ATTEN_DB_12;
        adc_pattern[i].channel = channel[i] & 0x7;
        adc_pattern[i].unit = IR_SENS_ADC_UNIT;
        adc_pattern[i].bit_width = ADC_BITWIDTH_12;

        ESP_LOGI(TAG, "adc_pattern[%d].atten is :%" PRIx8, i, adc_pattern[i].atten);
        ESP_LOGI(TAG, "adc_pattern[%d].channel is :%" PRIx8, i, adc_pattern[i].channel);
        ESP_LOGI(TAG, "adc_pattern[%d].unit is :%" PRIx8, i, adc_pattern[i].unit);
    }
    dig_cfg.adc_pattern = adc_pattern;
    ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));

    *out_handle = handle;
}

void measure_qre_on(ir_sens_value *values)
{
    gpio_set_level(IR_LED_POWER_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(IR_LED_RISE_MS));

    esp_err_t ret;
    uint32_t ret_num = 0;
    uint8_t adc_result[ADC_READ_LEN] = {0};
    memset(adc_result, 0xcc, ADC_READ_LEN);

    ir_sens_mux.channel(ir_sens_addresses[0]);
    ESP_ERROR_CHECK(adc_continuous_start(adc_handle));

    for (int i = 0; i < NUM_IR_SENS_ON_BOARD; i++)
    {
        int ir_sens_address = ir_sens_addresses[i];
        ir_sens_mux.channel(ir_sens_address);

        // while (1)
        // {
        //     // ESP_LOGI(TAG, "###measure_qre_off channel %d NOTIFIED", i);

        //     ret = adc_continuous_read(adc_handle, adc_result, ADC_READ_LEN, &ret_num, 0);
        //     if (ret_num < ADC_READ_LEN)
        //     {
        //         break;
        //     }
        // }
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        ret = adc_continuous_read(adc_handle, adc_result, ADC_READ_LEN, &ret_num, 0);

        // // Exchange buffers
        // uint8_t *temp_buf = adc_result;
        // adc_result = adc_result;
        // adc_result = temp_buf;

        // selected_ret_num = ret_num;
        // memcpy(adc_result, adc_result, sizeof(uint8_t) * ADC_READ_LEN);


        for (int j = 0; j < ret_num && (j / SOC_ADC_DIGI_RESULT_BYTES) < MULTISAMPLING_DEPTH; j += SOC_ADC_DIGI_RESULT_BYTES)
        {
            adc_digi_output_data_t *p = (adc_digi_output_data_t *)&adc_result[j];
            uint32_t chan_num = p->type2.channel;
            uint32_t data = p->type2.data;

            if (j == 0)
            {
                values[i].value_on = 0;
            }
            values[i].value_on += data;
        }
    }

    gpio_set_level(IR_LED_POWER_PIN, 1);
    ESP_ERROR_CHECK(adc_continuous_stop(adc_handle));

    for (int i = 0; i < NUM_IR_SENS_ON_BOARD; i++)
    {
        values[i].value_on = values[i].value_on / MULTISAMPLING_DEPTH;
    }
}

void measure_qre_off(ir_sens_value *values)
{
    ESP_LOGI(TAG, "###measure_qre_off");
    gpio_set_level(IR_LED_POWER_PIN, 1);

    esp_err_t ret;
    uint32_t ret_num = 0;
    uint8_t adc_result[ADC_READ_LEN] = {0};
    memset(adc_result, 0xcc, ADC_READ_LEN);

    ir_sens_mux.channel(ir_sens_addresses[0]);
    ESP_ERROR_CHECK(adc_continuous_start(adc_handle));

    for (int i = 0; i < NUM_IR_SENS_ON_BOARD; i++)
    {
        int ir_sens_address = ir_sens_addresses[i];
        ir_sens_mux.channel(ir_sens_address);

        
        // while (1)
        // {
        //     // ESP_LOGI(TAG, "###measure_qre_off channel %d NOTIFIED", i);

        //     ret = adc_continuous_read(adc_handle, adc_result, ADC_READ_LEN, &ret_num, 0);
        //     if (ret_num < ADC_READ_LEN)
        //     {
        //         break;
        //     }
        // }
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        ret = adc_continuous_read(adc_handle, adc_result, ADC_READ_LEN, &ret_num, 0);

        // // Exchange buffers
        // uint8_t *temp_buf = adc_result;
        // adc_result = adc_result;
        // adc_result = temp_buf;

        // selected_ret_num = ret_num;
        // memcpy(adc_result, adc_result, sizeof(uint8_t) * ADC_READ_LEN);
        

        for (int j = 0; j < ret_num && (j / SOC_ADC_DIGI_RESULT_BYTES) < MULTISAMPLING_DEPTH; j += SOC_ADC_DIGI_RESULT_BYTES)
        {
            adc_digi_output_data_t *p = (adc_digi_output_data_t *)&adc_result[j];
            uint32_t chan_num = p->type2.channel;
            uint32_t data = p->type2.data;

            if (j == 0)
            {
                values[i].value_off = 0;
            }
            values[i].value_off += data;
        }
    }

    gpio_set_level(IR_LED_POWER_PIN, 1);
    ESP_ERROR_CHECK(adc_continuous_stop(adc_handle));

    for (int i = 0; i < NUM_IR_SENS_ON_BOARD; i++)
    {
        values[i].value_off = values[i].value_off / MULTISAMPLING_DEPTH;
    }
}

void measure_ir_sens(ir_sens_value *values)
{
    measure_qre_off(values);

    uint32_t c0 = esp_cpu_get_cycle_count();
    measure_qre_on(values);
    printf(">cycle count:%lu\n", esp_cpu_get_cycle_count() - c0);

    for (int i = 0; i < NUM_IR_SENS_ON_BOARD; i++)
    {
        values[i].value_diff = values[i].value_off - values[i].value_on;
    }
}

void loop()
{
    gpio_set_level(IR_LED_POWER_PIN, 1);

    ir_sens_value sens_values[NUM_IR_SENS_ON_BOARD];
    measure_ir_sens(sens_values);

    for (int i = 0; i < NUM_IR_SENS_ON_BOARD; i++)
    {
        printf(">on");
        printf("%d", i);
        printf(":");
        printf("%4.0f", sens_values[i].value_on);
        printf("\n");
        printf(">off");
        printf("%d", i);
        printf(":");
        printf("%4.0f", sens_values[i].value_off);
        printf("\n");
        printf(">diff");
        printf("%d", i);
        printf(":");
        printf("%4.0f", sens_values[i].value_diff);
        printf("\n");
    }
    // print_new_marble_type(sens_value);

    // printf(">diff");
    // printf("%d", ir_sens_address);
    // printf(":");
    // printf("%4.0f", sens_value.value_diff);
    // printf("\n");

    // printf(">off");
    // printf("%d", ir_sens_address);
    // printf(":");
    // printf("%4.0f", sens_value.value_off);
    // printf("\n");

    // printf(">on");
    // printf("%d", ir_sens_address);
    // printf(":");
    // printf("%4.0f", sens_value.value_on);
    // printf("\n");
}

#ifdef __cplusplus
extern "C"
{
#endif

    void app_main(void)
    {
        s_task_handle = xTaskGetCurrentTaskHandle();

        continuous_adc_init(channel, sizeof(channel) / sizeof(adc_channel_t), &adc_handle);

        adc_continuous_evt_cbs_t cbs = {
            .on_conv_done = s_conv_done_cb,
        };
        ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(adc_handle, &cbs, NULL));

        gpio_reset_pin(IR_LED_POWER_PIN);
        gpio_set_direction(IR_LED_POWER_PIN, GPIO_MODE_OUTPUT);
        gpio_set_level(IR_LED_POWER_PIN, 1);

        while (1)
        {
            loop();
            vTaskDelay(pdMS_TO_TICKS(2000));
        }

        ESP_ERROR_CHECK(adc_continuous_deinit(adc_handle));
    }

#ifdef __cplusplus
}
#endif