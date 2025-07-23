#pragma once

#include "CD74HC4067.h"

#include <driver/gpio.h>

#define PUSH_BUTTONS_NUM 8
#define PUSH_BUTTONS_START_CHAN 8

class PushButtonsController
{
public:
    PushButtonsController(CD74HC4067 &mux, gpio_num_t mux_gpio);

    void update();

private:
    CD74HC4067 &_mux;
    gpio_num_t _mux_gpio;

    bool _push_button_states[PUSH_BUTTONS_NUM];
    uint8_t _click_events_pending[PUSH_BUTTONS_NUM];
};