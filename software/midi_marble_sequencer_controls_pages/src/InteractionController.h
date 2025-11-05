#pragma once

#include "controls_pages_common.h"
#include "CD74HC4067_arduino.h"
#include "PushButtonsController.h"
#include "IEventListener.h"


#define BUTTONS_MUX_SIGNAL_GPIO 26

#define BUTTONS_MUX_S0_GPIO 27
#define BUTTONS_MUX_S1_GPIO 32
#define BUTTONS_MUX_S2_GPIO 33
#define BUTTONS_MUX_S3_GPIO 25


class InteractionController
{
public:
    InteractionController();

    void update();
    controls_pages_value_t get_event();
    void consume_events(controls_pages_display_t controls_pages_display);
    void set_event_listener(IEventListener *event_listener);

private:
    IEventListener *_event_listener;
    CD74HC4067 _mux;

    PushButtonsController _played_pages_buttons_controller;
    PushButtonsController _edited_pages_buttons_controller;
};