#pragma once

#include "CD74HC4067.h"
#include "controls_main_common.h"

#include <driver/gpio.h>

#define PUSH_BUTTONS_NUM SEQUENCER_TRACKS_NUM
#define PUSH_BUTTONS_START_CHAN 8

class PushButtonsController
{
public:
    PushButtonsController(CD74HC4067 &mux, gpio_num_t mux_gpio);

    void update();
    push_button_event_t *get_push_buttons_events();
    void consume_events();

private:
    CD74HC4067 &_mux;
    gpio_num_t _mux_gpio;

    push_button_event_t _push_buttons_events[PUSH_BUTTONS_NUM];
};