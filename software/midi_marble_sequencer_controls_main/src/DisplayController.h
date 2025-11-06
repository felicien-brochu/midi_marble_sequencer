#pragma once

#include "LEDColumn.h"
#include "PlayPauseBPMLED.h"

#include "controls_main_common.h"

class DisplayController
{
public:
    DisplayController();

    void update(controls_main_display_t controls_main_display);

private:
    LEDColumn _tracks_leds;
    PlayPauseBPMLED _play_pause_led;
};