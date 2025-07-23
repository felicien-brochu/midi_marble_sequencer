#include "PushButtonsController.h"

PushButtonsController::PushButtonsController(CD74HC4067 &mux, gpio_num_t mux_gpio) : _mux(mux), _mux_gpio(mux_gpio)
{
    for (size_t i = 0; i < PUSH_BUTTONS_NUM; i++)
    {
        _push_button_states[i] = false;
        _click_events_pending[i] = 0;
    }
}

void PushButtonsController::update()
{
    for (size_t i = 0; i < PUSH_BUTTONS_NUM; i++)
    {
        _mux.channel(PUSH_BUTTONS_START_CHAN + i);
        bool push_button_level = (bool) gpio_get_level(_mux_gpio);

        if (push_button_level != _push_button_states[i])
        {
            _push_button_states[i] = push_button_level;

            if (!push_button_level)
            {
                _click_events_pending[i]++;
                printf("PUSH_BUTTON %d clicked %d times\n", i, _click_events_pending[i]);
            }
        }
    }
}