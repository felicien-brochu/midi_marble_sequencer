#pragma once

#include <driver/gpio.h>

#define PLAY_PAUSE_LED_GPIO GPIO_NUM_23

class PlayPauseLED
{
public:
    PlayPauseLED();

    bool is_enabled();
    void set_enabled(bool enabled);
    void enable();
    void disable();

private:
    bool _is_enabled;
};