#pragma once

#include "IRSensBoards.h"
#include "IRSensReader.h"
#include "PushButton.h"
#include "CalibrationDisplay.h"
#include "BeatsLEDSnake.h"

typedef enum
{
    BOARD_CALIBRATION_ACTIVE,
    BOARD_CALIBRATION_COMPLETE,
} board_calibration_state_t;

class BoardCalibrationPhase
{
public:
    BoardCalibrationPhase(IRSensBoards *ir_sens_boards, IRSensReader *board_reader, PushButton &push_button, CalibrationDisplay *display, BeatsLEDSnake *beats_led_snake);
    
    void update();
    bool is_complete();

private:
    IRSensBoards *_ir_sens_boards;
    IRSensReader *_board_reader;
    PushButton &_push_button;
    CalibrationDisplay *_display;
    BeatsLEDSnake *_beats_led_snake;

    board_calibration_state_t _state;
    uint8_t _selected_board_index;
    uint32_t _last_measurement_time;
    uint32_t _button_down_start_time;
    bool _button_was_down;
    bool _should_log_next_measurement;

    void _measure_and_display_sensors();
    void _update_led_line(uint8_t led_group, uint16_t value, uint16_t target, uint16_t min_acceptable, uint16_t max_acceptable);
    void _handle_button_events();
    void _select_next_board();
    void _on_calibration_complete();
};
