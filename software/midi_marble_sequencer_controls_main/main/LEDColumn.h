#pragma once

#include "controls_main_common.h"

#include <driver/gpio.h>

#define LED0_GPIO GPIO_NUM_27
#define LED1_GPIO GPIO_NUM_14
#define LED2_GPIO GPIO_NUM_13
#define LED3_GPIO GPIO_NUM_4
#define LED4_GPIO GPIO_NUM_16
#define LED5_GPIO GPIO_NUM_17
#define LED6_GPIO GPIO_NUM_18
#define LED7_GPIO GPIO_NUM_19


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
    gpio_num_t _led_gpios[SEQUENCER_TRACKS_NUM] = {LED0_GPIO, LED1_GPIO, LED2_GPIO, LED3_GPIO, LED4_GPIO, LED5_GPIO, LED6_GPIO, LED7_GPIO};
};