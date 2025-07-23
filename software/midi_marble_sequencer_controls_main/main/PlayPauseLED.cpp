#include "PlayPauseLED.h"

PlayPauseLED::PlayPauseLED()
{
    gpio_reset_pin(PLAY_PAUSE_LED_GPIO);
    gpio_set_direction(PLAY_PAUSE_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_pullup_dis(PLAY_PAUSE_LED_GPIO);
    gpio_pulldown_dis(PLAY_PAUSE_LED_GPIO);

    _is_enabled = false;
}

bool PlayPauseLED::is_enabled()
{
    return _is_enabled;
}

void PlayPauseLED::set_enabled(bool enabled)
{
    if (enabled != _is_enabled)
    {
        if (enabled)
        {
            enable();
        }
        else
        {
            disable();
        }
    }
}

void PlayPauseLED::enable()
{
    gpio_set_level(PLAY_PAUSE_LED_GPIO, 1);
}

void PlayPauseLED::disable()
{
    gpio_set_level(PLAY_PAUSE_LED_GPIO, 0);
}