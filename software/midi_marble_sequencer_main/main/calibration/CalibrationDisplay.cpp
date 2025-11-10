#include "CalibrationDisplay.h"
#include <cstring>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

CalibrationDisplay::CalibrationDisplay()
{
	// Initialize stored displays to safe defaults
	std::memset(&_controls_main_display, 0, sizeof(_controls_main_display));
	_controls_main_display.bpm = -1;

	std::memset(&_controls_pages_display, 0, sizeof(_controls_pages_display));

	// Create mutexes to protect access to each stored display
	_mutex_main = xSemaphoreCreateMutex();
	_mutex_pages = xSemaphoreCreateMutex();
	// If creation fails, the corresponding field will be NULL; code falls back to non-locked copy
}

CalibrationDisplay::~CalibrationDisplay()
{
	if (_mutex_main != NULL) {
		vSemaphoreDelete(_mutex_main);
		_mutex_main = NULL;
	}
	if (_mutex_pages != NULL) {
		vSemaphoreDelete(_mutex_pages);
		_mutex_pages = NULL;
	}
}

void CalibrationDisplay::handle_controls_main_event(controls_main_value_t controls_main_value)
{
	// Calibration display doesn't react to control events for now.
}

void CalibrationDisplay::handle_controls_pages_event(controls_pages_value_t controls_pages_value)
{
	// Calibration display doesn't react to page events for now.
}

void CalibrationDisplay::set_controls_main_display(const controls_main_display_t &controls_main_display)
{
	if (_mutex_main == NULL) {
		// Fallback: no mutex available, copy directly
		_controls_main_display = controls_main_display;
		return;
	}

	if (xSemaphoreTake(_mutex_main, portMAX_DELAY) == pdTRUE) {
		_controls_main_display = controls_main_display;
		xSemaphoreGive(_mutex_main);
	}
}

void CalibrationDisplay::set_controls_pages_display(const controls_pages_display_t &controls_pages_display)
{
	if (_mutex_pages == NULL) {
		_controls_pages_display = controls_pages_display;
		return;
	}

	if (xSemaphoreTake(_mutex_pages, portMAX_DELAY) == pdTRUE) {
		_controls_pages_display = controls_pages_display;
		xSemaphoreGive(_mutex_pages);
	}
}

void CalibrationDisplay::set_led_enabled(uint8_t led_group_index, uint8_t led_index, bool enabled)
{
    switch (led_group_index)
    {
        case 0:
            if (led_index < sizeof(_controls_main_display.tracks_led_enabled)) {
                _controls_main_display.tracks_led_enabled[led_index] = enabled;
            }
            break;
        case 1:
            if (led_index < sizeof(_controls_pages_display.edited_pages_led_enabled)) {
                _controls_pages_display.edited_pages_led_enabled[led_index] = enabled;
            }
            break;
        case 2:
            if (led_index < sizeof(_controls_pages_display.played_pages_led_enabled)) {
                _controls_pages_display.played_pages_led_enabled[led_index] = enabled;
            }
            break;
        default:
            // Invalid group index
            break;
    }
}

void CalibrationDisplay::set_led_group_enabled(uint8_t led_group_index, bool enabled)
{
    switch (led_group_index)
    {
        case 0:
            for (size_t i = 0; i < sizeof(_controls_main_display.tracks_led_enabled); i++)
            {
                _controls_main_display.tracks_led_enabled[i] = enabled;
            }
            break;
        case 1:
            for (size_t i = 0; i < sizeof(_controls_pages_display.edited_pages_led_enabled); i++)
            {
                _controls_pages_display.edited_pages_led_enabled[i] = enabled;
            }
            break;
        case 2:
            for (size_t i = 0; i < sizeof(_controls_pages_display.played_pages_led_enabled); i++)
            {
                _controls_pages_display.played_pages_led_enabled[i] = enabled;
            }
            break;
        default:
            // Invalid group index
            break;
    }
}

void CalibrationDisplay::show_completion_rate(float completion_rate)
{
    // Use the played pages LEDs to show completion rate
    uint8_t num_leds_to_light = static_cast<uint8_t>(completion_rate * SEQUENCER_PAGES_NUM);

    if (num_leds_to_light > SEQUENCER_PAGES_NUM) {
        num_leds_to_light = SEQUENCER_PAGES_NUM;
    }
    
    for (size_t i = 0; i < SEQUENCER_PAGES_NUM; i++)
    {
        _controls_pages_display.played_pages_led_enabled[i] = (i < num_leds_to_light);
    }
}

void CalibrationDisplay::hide_marble_placement()
{
    // Turn off all tracks LEDs
    set_led_group_enabled(0, false);
}

controls_main_display_t CalibrationDisplay::get_controls_main_display()
{
	controls_main_display_t copy;

	if (_mutex_main == NULL) {
		copy = _controls_main_display;
		return copy;
	}

	if (xSemaphoreTake(_mutex_main, portMAX_DELAY) == pdTRUE) {
		copy = _controls_main_display;
		xSemaphoreGive(_mutex_main);
	} else {
		// If we couldn't take the mutex, return a safe default
		std::memset(&copy, 0, sizeof(copy));
		copy.bpm = -1;
	}

	return copy;
}

controls_pages_display_t CalibrationDisplay::get_controls_pages_display()
{
	controls_pages_display_t copy;

	if (_mutex_pages == NULL) {
		copy = _controls_pages_display;
		return copy;
	}

	if (xSemaphoreTake(_mutex_pages, portMAX_DELAY) == pdTRUE) {
		copy = _controls_pages_display;
		xSemaphoreGive(_mutex_pages);
	} else {
		std::memset(&copy, 0, sizeof(copy));
	}

	return copy;
}