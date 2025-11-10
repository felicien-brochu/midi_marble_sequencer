#pragma once

#include "IControlBoardsListener.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>


class CalibrationDisplay : public IControlBoardsListener {
public:
    CalibrationDisplay();
    ~CalibrationDisplay();
    
    // IControlBoardsListener interface
    void handle_controls_main_event(controls_main_value_t controls_main_value) override;
    void handle_controls_pages_event(controls_pages_value_t controls_pages_value) override;
    controls_main_display_t get_controls_main_display() override;
    controls_pages_display_t get_controls_pages_display() override;

    // Thread-safe setters to update what the display should show from other tasks
    void set_controls_main_display(const controls_main_display_t &controls_main_display);
    void set_controls_pages_display(const controls_pages_display_t &controls_pages_display);

    void set_led_enabled(uint8_t led_group_index, uint8_t led_index, bool enabled);
    void set_led_group_enabled(uint8_t led_group_index, bool enabled);

    void show_completion_rate(float completion_rate);
    void hide_marble_placement();

private:
    // Stored display states (protected by _mutex)
    controls_main_display_t _controls_main_display;
    controls_pages_display_t _controls_pages_display;

    // One mutex per attribute to reduce contention
    SemaphoreHandle_t _mutex_main;
    SemaphoreHandle_t _mutex_pages;
};