#pragma once

#include "LEDRow.h"

#include "controls_pages_common.h"
#include <LEDArray.h>

#define LED_ARRAY_POWER_GPIO GPIO_NUM_15

#define LED_ARRAY_S0_GPIO GPIO_NUM_17
#define LED_ARRAY_S1_GPIO GPIO_NUM_16
#define LED_ARRAY_S2_GPIO GPIO_NUM_4
#define LED_ARRAY_S3_GPIO GPIO_NUM_2

class DisplayController
{
public:
    DisplayController();

    void update(controls_pages_display_t controls_pages_display);

private:
    LEDArray _led_array;

    LEDRow _played_led_row;
    LEDRow _edited_led_row;
};