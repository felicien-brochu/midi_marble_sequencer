#include "DisplayController.h"

DisplayController::DisplayController()
{
}

void DisplayController::update(controls_main_display_t controls_main_display)
{
    _play_pause_led.set_enabled(controls_main_display.play_pause_led_enabled);
    _tracks_leds.update_leds(controls_main_display.tracks_led_enabled);
}