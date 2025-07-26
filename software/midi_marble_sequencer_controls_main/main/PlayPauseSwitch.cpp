#include "PlayPauseSwitch.h"

PlayPauseSwitch::PlayPauseSwitch()
{
    gpio_reset_pin(PLAY_PAUSE_SWITCH_GPIO);
    gpio_set_direction(PLAY_PAUSE_SWITCH_GPIO, GPIO_MODE_INPUT);
    gpio_pullup_dis(PLAY_PAUSE_SWITCH_GPIO);
    gpio_pulldown_dis(PLAY_PAUSE_SWITCH_GPIO);

    _is_pushed = false;
}

void PlayPauseSwitch::update()
{
    bool switch_level = (bool) gpio_get_level(PLAY_PAUSE_SWITCH_GPIO);

    if (switch_level != _is_pushed)
    {
        _is_pushed = switch_level;
    }
}

bool PlayPauseSwitch::is_pushed()
{
    return _is_pushed;
}
