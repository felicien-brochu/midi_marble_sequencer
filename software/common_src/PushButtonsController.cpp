#include "PushButtonsController.h"
#include <freertos/FreeRTOS.h>
#include <cstring>
#include <esp32-hal-log.h>
#include <Arduino.h>

static const char *TAG = "PushButtonsController";

PushButtonsController::PushButtonsController(CD74HC4067 &mux, uint8_t mux_gpio, const uint8_t *buttons_indexes, uint8_t num_buttons) : _mux(mux), _mux_gpio(mux_gpio)
{
    _num_buttons = num_buttons;
    _buttons_indexes = buttons_indexes;

    // GPIO settings must be set after ADC initialization
    // which takes place in RotaryButtonsController
    pinMode(_mux_gpio, INPUT);

    for (size_t i = 0; i < PUSH_BUTTONS_CONTROLLER_MAX_BTN; i++)
    {
        _push_buttons_events[i].pushed = false;
        _push_buttons_events[i].click_events_pending = 0;
    }
}

bool PushButtonsController::update()
{
    pinMode(_mux_gpio, INPUT);

    bool has_changed = false;

    for (size_t i = 0; i < _num_buttons; i++)
    {
        _mux.channel(_buttons_indexes[i]);
        bool push_button_level = (bool) digitalRead(_mux_gpio) == HIGH;

        if (push_button_level != _push_buttons_events[i].pushed)
        {
            _push_buttons_events[i].pushed = push_button_level;
            has_changed = true;

            if (!push_button_level)
            {
                _push_buttons_events[i].click_events_pending++;
                // log_i("Button click [%d] pending: %d", i, _push_buttons_events[i].click_events_pending);
            }
        }
    }

    return has_changed;
}

void PushButtonsController::get_push_buttons_events(push_button_event_t *event_buffer)
{
    memcpy(event_buffer, _push_buttons_events, _num_buttons * sizeof(push_button_event_t));
}

uint16_t PushButtonsController::get_push_buttons_events_flags()
{
    uint16_t buttons_events_flags = 0;

    for (size_t i = 0; i < PUSH_BUTTONS_CONTROLLER_MAX_BTN; i++)
    {
        buttons_events_flags = buttons_events_flags << 1;
        buttons_events_flags |= (_push_buttons_events[i].click_events_pending % 2);
        buttons_events_flags = buttons_events_flags << 1;
        buttons_events_flags |= _push_buttons_events[i].pushed;
    }
    
    return buttons_events_flags;
}

void PushButtonsController::consume_events(uint8_t consumed_clicks)
{
    log_i("consumed_clicks bits: %x", consumed_clicks);
    for (int i = _num_buttons - 1; i >= 0; i--)
    {
        if (consumed_clicks & 0x01) {
            log_i("Consume button[%d]", i);
            _push_buttons_events[i].click_events_pending = (_push_buttons_events[i].click_events_pending - 1) % 2;
        }
        else {
            _push_buttons_events[i].click_events_pending = _push_buttons_events[i].click_events_pending % 2;
        }
        consumed_clicks = consumed_clicks >> 1;
    }
}
