#include "PlayPauseSwitch.h"

PlayPauseSwitch::PlayPauseSwitch()
{
    gpio_reset_pin(PLAY_PAUSE_SWITCH_GPIO);
    gpio_set_direction(PLAY_PAUSE_SWITCH_GPIO, GPIO_MODE_INPUT);
    gpio_pullup_dis(PLAY_PAUSE_SWITCH_GPIO);
    gpio_pulldown_dis(PLAY_PAUSE_SWITCH_GPIO);

    _is_pressed = false;
}

void PlayPauseSwitch::update()
{
    bool switch_level = (bool) gpio_get_level(PLAY_PAUSE_SWITCH_GPIO);

    if (switch_level != _is_pressed)
    {
        _is_pressed = switch_level;

        printf("PlayPauseSwitch changed state: %d\n", switch_level);
    }
}
