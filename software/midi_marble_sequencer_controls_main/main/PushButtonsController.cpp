#include "PushButtonsController.h"

PushButtonsController::PushButtonsController(CD74HC4067 &mux, gpio_num_t mux_gpio) : _mux(mux), _mux_gpio(mux_gpio)
{
    // GPIO settings must be set after ADC initialization
    // which takes place in RotaryButtonsController
    gpio_reset_pin(mux_gpio);
    gpio_set_direction(mux_gpio, GPIO_MODE_INPUT);
    gpio_pullup_dis(mux_gpio);
    gpio_pulldown_dis(mux_gpio);

    for (size_t i = 0; i < PUSH_BUTTONS_NUM; i++)
    {
        _push_buttons_events[i].pushed = false;
        _push_buttons_events[i].click_events_pending = 0;
    }
}

void PushButtonsController::update()
{
    for (size_t i = 0; i < PUSH_BUTTONS_NUM; i++)
    {
        _mux.channel(PUSH_BUTTONS_START_CHAN + i);
        bool push_button_level = (bool) gpio_get_level(_mux_gpio);

        if (push_button_level != _push_buttons_events[i].pushed)
        {
            printf("####PUSH_BUTTONS[%d] : %d\n", i, push_button_level);
            _push_buttons_events[i].pushed = push_button_level;

            if (!push_button_level)
            {
                _push_buttons_events[i].click_events_pending++;
            }
        }
    }
}

push_button_event_t *PushButtonsController::get_push_buttons_events()
{
    return _push_buttons_events;
}

void PushButtonsController::consume_events()
{
    for (size_t i = 0; i < PUSH_BUTTONS_NUM; i++)
    {
        _push_buttons_events[i].click_events_pending = 0;
    }
}