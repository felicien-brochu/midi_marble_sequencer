#pragma once

#include "controls_main_common.h"

#define LED0_GPIO 27
#define LED1_GPIO 14
#define LED2_GPIO 13
#define LED3_GPIO 4
#define LED4_GPIO 16
#define LED5_GPIO 17
#define LED6_GPIO 18
#define LED7_GPIO 19


class LEDColumn
{
public:
    LEDColumn();

    
    void update_leds(bool *leds_enabled);
    void enable_led(uint8_t index);
    void disable_led(uint8_t index);
    void enable_all_leds();
    void disable_all_leds();

private:
    uint8_t _led_gpios[SEQUENCER_TRACKS_NUM] = {LED0_GPIO, LED1_GPIO, LED2_GPIO, LED3_GPIO, LED4_GPIO, LED5_GPIO, LED6_GPIO, LED7_GPIO};
};