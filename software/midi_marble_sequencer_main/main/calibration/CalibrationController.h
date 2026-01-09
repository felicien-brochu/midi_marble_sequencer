#include "IRSensBoards.h"
#include "IRSensReader.h"
#include "PushButton.h"
#include "marble_type.h"
#include "CalibrationDisplay.h"
#include "ControlBoardsController.h"
#include "SensorStatistics.h"
#include "BoardCalibrationPhase.h"
#include "BeatsLEDSnake.h"

typedef enum
{
    CALIBRATION_BOARD_PHASE,
    CALIBRATION_IDLE,
    CALIBRATION_WAIT_PLACEMENT,
    CALIBRATION_READ,
    CALIBRATION_COMPLETE,
} calibration_state_t;

class CalibrationController
{
public:

    CalibrationController(int marbles_for_one_color, int samples_by_test, int ms_between_samples);

    void update();
    bool is_complete();
    
    private:
    int _marbles_for_one_color;
    int _samples_by_test;
    int _ms_between_samples;

    IRSensBoards _ir_sens_boards;
    IRSensReader _board_reader;
    PushButton &_push_button;
    CalibrationDisplay _display;
    ControlBoardsController _control_boards_controller;
    BeatsLEDSnake _beats_led_snake;
    BoardCalibrationPhase *_board_calibration_phase;

    uint32_t *_values_on;
    uint32_t *_values_off;

    calibration_state_t _calibration_state;
    uint16_t _measure_count;
    SensorStatistics **_statistics;

    void _update_board_calibration_phase();
    void _update_idle_state();
    void _update_waiting_placement_state();
    void _update_read_state();
    void _on_measure_complete();
    void _sensor_statistics_to_thresholds(uint16_t (*out_thresholds)[NUM_IR_SENS_BY_BOARD][NUM_MARBLE_TYPE - 1]);
    void _save_calibration_data_to_nvs();
    bool _ask_confirmation_to_save();
    int _get_total_measure_count();
    int _get_num_marble_types_groups();
    void _print_marble_placement();
    void _print_statistics();
    void _print_csv_data();
    void _print_c_array_thresholds();
    void _print_c_array_means();
    void _print_marble_intervals_for_sensor(uint8_t board_index, uint8_t ir_sens_channel);
    marble_type_t _get_ir_sens_marble_type(uint8_t ir_sens_channel);
};