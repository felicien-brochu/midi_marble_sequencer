#pragma once

#include "LEDRow.h"

#include "controls_pages_common.h"
#include <LEDArray_arduino.h>

#define LED_ARRAY_POWER_GPIO 15

#define LED_ARRAY_S0_GPIO 17
#define LED_ARRAY_S1_GPIO 16
#define LED_ARRAY_S2_GPIO 4
#define LED_ARRAY_S3_GPIO 2

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