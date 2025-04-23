#include "CD74HC4067.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

typedef enum
{
    NO_MARBLE,
    BLACK_MARBLE,
    BLUE_MARBLE,
    BEIGE_MARBLE,
    WHITE_MARBLE,
} marble_type_t;

#define IR_LED_RISE_MS 10
#define IR_LED_POWER_PIN GPIO_NUM_14
// ESP-32-S3 GPIO 4 (found in pinout)
#define IR_SENS_ADC_UNIT ADC_UNIT_1
#define IR_SENS_ADC_CHANNEL ADC_CHANNEL_3

// ESP32-WROOM-32D GPIO 32
// #define IR_SENS_ADC_UNIT ADC_UNIT_1
// #define IR_SENS_ADC_CHANNEL ADC_CHANNEL_4

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

struct ir_sens_value {
    double value_on;
    double value_off;
    double value_diff;
};

CD74HC4067 ir_sens_mux(MUX_S0_PIN, MUX_S1_PIN, MUX_S2_PIN, MUX_S3_PIN);


int current_marble_type = NO_MARBLE;

static int adc_raw[10];
adc_oneshot_unit_handle_t adc_handle;
adc_oneshot_chan_cfg_t adc_channel_config = {
    .atten = ADC_ATTEN_DB_12,
    .bitwidth = ADC_BITWIDTH_DEFAULT,
};

double measure_qre_on()
{
    gpio_set_level(IR_LED_POWER_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(IR_LED_RISE_MS));

    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, IR_SENS_ADC_CHANNEL, &adc_raw[0]));
    double qre_value_on = adc_raw[0];
    gpio_set_level(IR_LED_POWER_PIN, 0);
    return qre_value_on;
}

double measure_qre_off()
{
    gpio_set_level(IR_LED_POWER_PIN, 0);
    // uint32_t c0 = esp_cpu_get_cycle_count();
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, IR_SENS_ADC_CHANNEL, &adc_raw[0]));
    // printf(">cycle count:%lu\n", esp_cpu_get_cycle_count() - c0);
    double qre_value_off = adc_raw[0];
    return qre_value_off;
}

double multisample(int nb_sample, double (*measuring_func)())
{
    double value = 0;
    for (int i = 0; i < nb_sample; i++)
    {
        value += measuring_func();
    }

    return value / nb_sample;
}

ir_sens_value measure_ir_sens()
{
    ir_sens_value value;

    value.value_off = multisample(5, &measure_qre_off);
    value.value_on = multisample(5, &measure_qre_on);
    value.value_diff = value.value_off - value.value_on;

    return value;
}

marble_type_t getMarbleType(ir_sens_value value)
{
    double diff = value.value_off - value.value_on;
    marble_type_t marble_type;
    if (diff > 3500)
    {
        marble_type = WHITE_MARBLE;
    }
    else if (diff > 2000)
    {
        marble_type = BEIGE_MARBLE;
    }
    else if (diff > 1500)
    {
        marble_type = BLUE_MARBLE;
    }
    else if (diff > 1050)
    {
        marble_type = BLACK_MARBLE;
    }
    else
    {
        marble_type = NO_MARBLE;
    }
    return marble_type;
}

void print_new_marble_type(const ir_sens_value &sens_value);

void loop()
{
    gpio_set_level(IR_LED_POWER_PIN, 0);
    int ir_sens_addresses[] = {0, 1, 2, 13, 14, 15};
    int nb_ir_sens = 1;

    for (int i = 0; i < nb_ir_sens; i++)
    {
        int ir_sens_address = ir_sens_addresses[i];
        ir_sens_mux.channel(ir_sens_address);
        ir_sens_value sens_value = measure_ir_sens();

        print_new_marble_type(sens_value);

        printf(">diff");
        printf("%d", ir_sens_address);
        printf(":");
        printf("%4.0f", sens_value.value_diff);
        printf("\n");

        printf(">off");
        printf("%d", ir_sens_address);
        printf(":");
        printf("%4.0f", sens_value.value_off);
        printf("\n");

        printf(">on");
        printf("%d", ir_sens_address);
        printf(":");
        printf("%4.0f", sens_value.value_on);
        printf("\n");
    }
}

void print_new_marble_type(const ir_sens_value &sens_value)
{
    marble_type_t marble_type = getMarbleType(sens_value);
    if (marble_type != current_marble_type)
    {
        current_marble_type = marble_type;

        switch (current_marble_type)
        {
        case NO_MARBLE:
            printf("NO_MARBLE\n");
            break;
        case BLACK_MARBLE:
            printf("BLACK_MARBLE\n");
            break;
        case BLUE_MARBLE:
            printf("BLUE_MARBLE\n");
            break;
        case BEIGE_MARBLE:
            printf("BEIGE_MARBLE\n");
            break;
        case WHITE_MARBLE:
            printf("WHITE_MARBLE\n");
            break;
        }
    }
}

#ifdef __cplusplus
extern "C"
{
#endif

void app_main()
{
    gpio_reset_pin(IR_LED_POWER_PIN);
    gpio_set_direction(IR_LED_POWER_PIN, GPIO_MODE_OUTPUT);
    // gpio_reset_pin(IR_SENS_PIN);
    // gpio_set_direction(IR_SENS_PIN, GPIO_MODE_INPUT);

    //-------------ADC Init---------------//
    adc_oneshot_unit_init_cfg_t adc_init_config = {
        .unit_id = IR_SENS_ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_init_config, &adc_handle));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, IR_SENS_ADC_CHANNEL, &adc_channel_config));

    gpio_set_level(IR_LED_POWER_PIN, 0);

    while (1)
    {
        loop();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

#ifdef __cplusplus
}
#endif