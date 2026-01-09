#include "BoardCalibrationPhase.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

const static char *TAG = "BoardCalibrationPhase";

// Sensor 0 (brown marble) calibration targets
#define SENSOR_0_TARGET 450
#define SENSOR_0_MIN_ACCEPTABLE 250
#define SENSOR_0_MAX_ACCEPTABLE 650

// Sensor 15 (blue marble, NUM_MARBLE_TYPE - 2) calibration targets
#define SENSOR_15_TARGET 2400
#define SENSOR_15_MIN_ACCEPTABLE 2000
#define SENSOR_15_MAX_ACCEPTABLE 2800

// Timing constants
#define MEASUREMENT_INTERVAL_MS 300
#define LONG_PRESS_DURATION_MS 5000

BoardCalibrationPhase::BoardCalibrationPhase(
    IRSensBoards *ir_sens_boards,
    IRSensReader *board_reader,
    PushButton &push_button,
    CalibrationDisplay *display,
    BeatsLEDSnake *beats_led_snake)
    : _push_button(push_button)
{
    _ir_sens_boards = ir_sens_boards;
    _board_reader = board_reader;
    _display = display;
    _beats_led_snake = beats_led_snake;

    _state = BOARD_CALIBRATION_ACTIVE;
    _selected_board_index = 0;
    _last_measurement_time = 0;
    _button_down_start_time = 0;
    _button_was_down = false;
    _should_log_next_measurement = true;

    // Initialize display - turn off all LEDs initially
    _display->set_led_group_enabled(1, false);  // edited_pages_led (blue marble)
    _display->set_led_group_enabled(2, false);  // played_pages_led (brown marble)

    // Enable the first board LED on the BeatsLEDSnake
    _beats_led_snake->set_eighth_note_index(_selected_board_index * 2);
    _beats_led_snake->set_enabled(true);
    _beats_led_snake->update();

    ESP_LOGI(TAG, "Board calibration phase started. Board 0 selected.");
    printf("=== BOARD CALIBRATION PHASE ===\n");
    printf("Place a BROWN marble on sensor 0 of each board.\n");
    printf("Place a BLUE marble on sensor 15 of each board.\n");
    printf("Use the potentiometer on the board to adjust the range.\n");
    printf("Short press: select next board\n");
    printf("Long press (5s): complete board calibration and move to sensor calibration\n");
}

void BoardCalibrationPhase::update()
{
    if (_state == BOARD_CALIBRATION_COMPLETE) {
        return;
    }

    _handle_button_events();

    // Perform measurements every 300ms
    TickType_t current_time = xTaskGetTickCount();
    if (current_time - _last_measurement_time >= pdMS_TO_TICKS(MEASUREMENT_INTERVAL_MS)) {
        _last_measurement_time = current_time;
        _measure_and_display_sensors();
    }
}

bool BoardCalibrationPhase::is_complete()
{
    return _state == BOARD_CALIBRATION_COMPLETE;
}

void BoardCalibrationPhase::_measure_and_display_sensors()
{
    uint32_t value_off_0, value_on_0;
    uint32_t value_off_15, value_on_15;

    // Read sensor 0 from the selected board
    _board_reader->read_sensor_value(&value_off_0, &value_on_0, _selected_board_index, 0, IR_SENSOR_MULTISAMPLING);
    
    // Read sensor 15 from the selected board
    _board_reader->read_sensor_value(&value_off_15, &value_on_15, _selected_board_index, 15, IR_SENSOR_MULTISAMPLING);

    // Calculate sensor difference (off - on) to detect marble presence
    uint16_t sensor_0_value = static_cast<uint16_t>(value_off_0 - value_on_0);
    uint16_t sensor_15_value = static_cast<uint16_t>(value_off_15 - value_on_15);

    // Update LED displays for sensor 0 (brown marble) - played_pages_led
    _update_led_line(2, sensor_0_value, SENSOR_0_TARGET, SENSOR_0_MIN_ACCEPTABLE, SENSOR_0_MAX_ACCEPTABLE);

    // Update LED displays for sensor 15 (blue marble) - edited_pages_led
    _update_led_line(1, sensor_15_value, SENSOR_15_TARGET, SENSOR_15_MIN_ACCEPTABLE, SENSOR_15_MAX_ACCEPTABLE);

    if (_should_log_next_measurement) {
        ESP_LOGI(TAG, "Board %d: Sensor 0=%d, Sensor 15=%d", _selected_board_index, sensor_0_value, sensor_15_value);
        _should_log_next_measurement = false;
    }
}

void BoardCalibrationPhase::_update_led_line(uint8_t led_group, uint16_t value, uint16_t target, uint16_t min_acceptable, uint16_t max_acceptable)
{
    // LED line has 8 LEDs (SEQUENCER_PAGES_NUM = 8)
    // LED mapping:
    // LED 0: value < min_acceptable
    // LEDs 1, 2, or 3 (individually): min_acceptable <= value < (target - half_tolerance)
    // LEDs 3 and 4 (both): (target - half_tolerance) <= value <= (target + half_tolerance)
    // LEDs 4, 5, or 6 (individually): (target + half_tolerance) < value <= max_acceptable
    // LED 7: value > max_acceptable

    // First, turn off all LEDs
    for (uint8_t i = 0; i < 8; i++) {
        _display->set_led_enabled(led_group, i, false);
    }

    if (value < min_acceptable) {
        // Only LED 0
        _display->set_led_enabled(led_group, 0, true);
    } 
    else if (value > max_acceptable) {
        // Only LED 7
        _display->set_led_enabled(led_group, 7, true);
    } 
    else {
        // Calculate tolerance zones
        uint16_t tolerance = (max_acceptable - min_acceptable) / 20;  // 5% of range
        uint16_t half_tolerance = tolerance / 2;
        
        if (value >= (target - half_tolerance) && value <= (target + half_tolerance)) {
            // Very close to target - light up LEDs 3 and 4
            _display->set_led_enabled(led_group, 3, true);
            _display->set_led_enabled(led_group, 4, true);
        } 
        else if (value < target) {
            // Below target
            // Between min_acceptable and (target - half_tolerance) - use LEDs 1, 2, 3
            uint16_t range = target - half_tolerance - min_acceptable;
            uint16_t led_range = range / 3;
            uint8_t led_index = 1 + (value - min_acceptable) / led_range;  // Maps to LEDs 1, 2, or 3
            if (led_index > 3) led_index = 3;
            _display->set_led_enabled(led_group, led_index, true);
        } 
        else {
            // Above target
            // Between (target + half_tolerance) and max_acceptable - use LEDs 4, 5, 6
            uint16_t range = max_acceptable - (target + half_tolerance);
            uint16_t led_range = range / 3;
            uint8_t led_index = 4 + (value - (target + half_tolerance)) / led_range;  // Maps to LEDs 4, 5, or 6
            if (led_index > 6) led_index = 6;
            _display->set_led_enabled(led_group, led_index, true);
        }
    }
}

void BoardCalibrationPhase::_handle_button_events()
{
    _push_button.update();

    if (_push_button.is_down()) {
        if (!_button_was_down) {
            // Button just went down
            _button_down_start_time = xTaskGetTickCount();
            _button_was_down = true;
        } else {
            // Button is held down - check if it's been 5 seconds
            TickType_t hold_duration = xTaskGetTickCount() - _button_down_start_time;
            if (hold_duration >= pdMS_TO_TICKS(LONG_PRESS_DURATION_MS)) {
                // Long press detected - complete board calibration phase
                ESP_LOGI(TAG, "Long press detected. Completing board calibration phase.");
                _on_calibration_complete();
                _button_was_down = false;
            }
        }
    } else {
        if (_button_was_down) {
            // Button just went up - this is a short press (click)
            TickType_t hold_duration = xTaskGetTickCount() - _button_down_start_time;
            if (hold_duration < pdMS_TO_TICKS(LONG_PRESS_DURATION_MS)) {
                // Short press - select next board
                _select_next_board();
            }
            _button_was_down = false;
        }
    }
}

void BoardCalibrationPhase::_select_next_board()
{
    _selected_board_index = (_selected_board_index + 1) % NUM_IR_SENS_BOARDS;

    // Update BeatsLEDSnake to show the newly selected board
    _beats_led_snake->set_eighth_note_index(_selected_board_index * 2);
    _beats_led_snake->update();

    ESP_LOGI(TAG, "Board %d selected", _selected_board_index);
    
    // Trigger logging on next measurement
    _should_log_next_measurement = true;
}

void BoardCalibrationPhase::_on_calibration_complete()
{
    printf("Board calibration phase complete. Waiting for button release...\n");

    // Wait for button to be released before ending the phase
    while (_push_button.is_down()) {
        _push_button.update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    // Clear any pending click events from the button release
    _push_button.update();
    if (_push_button.has_click_event_pending()) {
        _push_button.click_event_accounted_for();
    }
    
    printf("Button released. Moving to sensor calibration...\n");

    // Clean up display
    _display->set_led_group_enabled(1, false);
    _display->set_led_group_enabled(2, false);
    _beats_led_snake->set_enabled(false);
    _beats_led_snake->update();

    _state = BOARD_CALIBRATION_COMPLETE;
}
