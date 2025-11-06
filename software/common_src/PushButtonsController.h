#pragma once

#include "CD74HC4067_arduino.h"

#define PUSH_BUTTONS_CONTROLLER_MAX_BTN 8

typedef struct {
    bool pushed;
    uint8_t click_events_pending;
} push_button_event_t;

class PushButtonsController
{
public:
    PushButtonsController(CD74HC4067 &mux, uint8_t mux_gpio, const uint8_t *buttons_indexes, uint8_t num_buttons);

    bool update();
    void get_push_buttons_events(push_button_event_t *event_buffer);
    uint16_t get_push_buttons_events_flags();
    void consume_events(uint8_t consumed_clicks);

private:
    CD74HC4067 &_mux;
    uint8_t _mux_gpio;
    uint8_t _num_buttons;
    const uint8_t *_buttons_indexes;

    push_button_event_t _push_buttons_events[PUSH_BUTTONS_CONTROLLER_MAX_BTN];
};