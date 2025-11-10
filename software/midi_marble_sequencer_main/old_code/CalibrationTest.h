#include "IRSensBoards.h"
#include "IRSensReader.h"
#include "PushButton.h"
#include "marble_type.h"

typedef enum
{
    CALIBRATION_TEST_IDLE,
    CALIBRATION_TEST_WAIT_PLACEMENT,
    CALIBRATION_TEST_READ,
    CALIBRATION_TEST_COMPLETE,
} calibration_test_state_t;


class CalibrationTest
{
public:

    CalibrationTest(int marbles_for_one_color, int samples_by_test, int ms_between_samples, int multisampling);

    void update();

private:
    int _marbles_for_one_color;
    int _samples_by_test;
    int _ms_between_samples;
    int _multisampling;

    IRSensBoards _ir_sens_boards;
    IRSensReader _board_reader;
    PushButton _push_button;

    uint32_t *_values_on;
    uint32_t *_values_off;

    calibration_test_state_t _calibration_state;
    uint16_t _measure_count;

    void _idle_state_update();
    void _waiting_placement_state_update();
    void _read_state_update();
    void _print_marble_placement();
    void _test_board_sample(uint32_t *values_off, uint32_t *values_on, uint8_t board_index);
    marble_type_t _get_ir_sens_marble_type(uint8_t ir_sens_channel);
};