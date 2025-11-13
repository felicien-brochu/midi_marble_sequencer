#include "DisplayController.h"

static const uint8_t _played_led_indexes[] = {7, 6, 5, 4, 3, 2, 1, 0};
static const uint8_t _edited_led_indexes[] = {8, 9, 10, 11, 12, 13, 14, 15};

DisplayController::DisplayController() : _led_array(16, LED_ARRAY_POWER_GPIO, LED_ARRAY_S0_GPIO, LED_ARRAY_S1_GPIO, LED_ARRAY_S2_GPIO, LED_ARRAY_S3_GPIO), _played_led_row(_led_array, 8, _played_led_indexes), _edited_led_row(_led_array, 8, _edited_led_indexes)
{
}

void DisplayController::update(controls_pages_display_t controls_pages_display)
{
    _played_led_row.set_led_states(controls_pages_display.played_pages_led_states);
    _edited_led_row.set_led_states(controls_pages_display.edited_pages_led_states);
}