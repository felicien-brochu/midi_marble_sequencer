#include "PushButton.h"
#include <driver/gpio.h>

PushButton::PushButton(gpio_num_t gpio_num, bool one_is_up)
{
    _gpio_num = gpio_num;
    _one_is_up = one_is_up;

    gpio_config_t boot_button_config = {
        pin_bit_mask : 1ULL << _gpio_num,
        mode : GPIO_MODE_INPUT,
        pull_up_en : GPIO_PULLUP_ENABLE,
        pull_down_en : GPIO_PULLDOWN_DISABLE,
        intr_type : GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&boot_button_config));

    _push_button_state = _get_current_state();
}

void PushButton::update()
{
    push_button_state_t current_state = _get_current_state();

    if (_has_click_listener && _push_button_state == PUSH_BUTTON_DOWN && current_state == PUSH_BUTTON_UP) {
        _click_event_pending = true;
    }

    _push_button_state = current_state;
}

bool PushButton::is_up()
{
    return _push_button_state == PUSH_BUTTON_UP;
}

bool PushButton::is_down()
{
    return _push_button_state == PUSH_BUTTON_DOWN;
}

void PushButton::start_listening_clicks()
{
    _has_click_listener = true;
}

void PushButton::stop_listening_clicks()
{
    _has_click_listener = false;
    _click_event_pending = false;
}

bool PushButton::has_click_listener()
{
    return _has_click_listener;
}

bool PushButton::has_click_event_pending()
{
    return _click_event_pending;
}

void PushButton::click_event_accounted_for()
{
    _click_event_pending = false;
}

push_button_state_t PushButton::_get_current_state()
{
    int push_button_level = gpio_get_level(_gpio_num);

    return (_one_is_up && push_button_level) || (!_one_is_up && !push_button_level) ? PUSH_BUTTON_UP : PUSH_BUTTON_DOWN;
}
