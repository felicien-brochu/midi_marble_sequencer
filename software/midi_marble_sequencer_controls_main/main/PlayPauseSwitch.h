#pragma once

#include <driver/gpio.h>

#define PLAY_PAUSE_SWITCH_GPIO GPIO_NUM_39

class PlayPauseSwitch
{
public:
    PlayPauseSwitch();

    void update();
    bool is_pushed();

private:
    bool _is_pushed;
};