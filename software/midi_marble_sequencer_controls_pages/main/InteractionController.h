#pragma once

#include "controls_pages_common.h"
#include "CD74HC4067.h"
#include "PushButtonsController.h"


#define BUTTONS_MUX_SIGNAL_GPIO GPIO_NUM_26

#define BUTTONS_MUX_S0_GPIO GPIO_NUM_27
#define BUTTONS_MUX_S1_GPIO GPIO_NUM_32
#define BUTTONS_MUX_S2_GPIO GPIO_NUM_33
#define BUTTONS_MUX_S3_GPIO GPIO_NUM_25


class InteractionController
{
public:
    InteractionController();

    void update();
    void get_value(controls_pages_value_t *value);
    void consume_events();

// private:
    CD74HC4067 _mux;

    PushButtonsController _played_pages_buttons_controller;
    PushButtonsController _edited_pages_buttons_controller;
};