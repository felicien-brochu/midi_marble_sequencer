#include "CalibrationTest.h"
#include "SensorsCalibrationData.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

CalibrationTest::CalibrationTest(int marbles_for_one_color, int samples_by_test, int ms_between_samples, int multisampling) : _board_reader(&_ir_sens_boards), _push_button(GPIO_NUM_3, false)
{
    _marbles_for_one_color = marbles_for_one_color;
    _samples_by_test = samples_by_test;
    _ms_between_samples = ms_between_samples;
    _multisampling = multisampling;

    _calibration_state = CALIBRATION_TEST_IDLE;
    _measure_count = 0;
    
    _values_on = (uint32_t*) malloc(NUM_IR_SENS_BY_BOARD * sizeof(uint32_t));
    _values_off = (uint32_t*) malloc(NUM_IR_SENS_BY_BOARD * sizeof(uint32_t));

    vTaskDelay(pdMS_TO_TICKS(3000));
}

void CalibrationTest::update()
{
    if (_calibration_state == CALIBRATION_TEST_IDLE)
    {
        _idle_state_update();
    }
    else if (_calibration_state == CALIBRATION_TEST_WAIT_PLACEMENT)
    {
        _waiting_placement_state_update();
    }
    if (_calibration_state == CALIBRATION_TEST_READ)
    {
        _read_state_update();
    }
}

void CalibrationTest::_idle_state_update()
{
    _print_marble_placement();

    _calibration_state = CALIBRATION_TEST_WAIT_PLACEMENT;
}

void CalibrationTest::_waiting_placement_state_update()
{
    if (!_push_button.has_click_listener()) {
        _push_button.start_listening_clicks();
    }

    _push_button.update();

    if (_push_button.has_click_event_pending()) {
        _push_button.click_event_accounted_for();
        _push_button.stop_listening_clicks();

        _calibration_state = CALIBRATION_TEST_READ;

        printf("Measuring...\n");
    }
}

void CalibrationTest::_read_state_update()
{
    for (size_t board_index = 0; board_index < NUM_IR_SENS_BOARDS; board_index++)
    {
        for (int i = 0; i < _samples_by_test; i++) {
            _board_reader.read_board_values(_values_off, _values_on, board_index, _multisampling);

            _test_board_sample(_values_off, _values_on, board_index);
            vTaskDelay(pdMS_TO_TICKS(_ms_between_samples));
        }
    }
        
    
    _measure_count++;

    if (_measure_count >= (NUM_IR_SENS_BY_BOARD / 2) * _marbles_for_one_color)
    {
        _calibration_state = CALIBRATION_TEST_COMPLETE;
    }
    else
    {
        _calibration_state = CALIBRATION_TEST_IDLE;
    }
}

void CalibrationTest::_test_board_sample(uint32_t *values_off, uint32_t *values_on, uint8_t board_index)
{
    for (size_t sensor_index = 0; sensor_index < NUM_IR_SENS_BY_BOARD; sensor_index++)
    {
        marble_type_t expected_marble_type = _get_ir_sens_marble_type(sensor_index);

        uint16_t low_threshold = 0;
        uint16_t high_threshold = 1 << 12;
        
        if (expected_marble_type > 0) {
            low_threshold = marble_types_thresholds[board_index][sensor_index][expected_marble_type - 1];
        }
        if (expected_marble_type < NUM_MARBLE_TYPE - 1) {
            high_threshold = marble_types_thresholds[board_index][sensor_index][expected_marble_type];
        }

        uint16_t diff = values_off[sensor_index] - values_on[sensor_index];

        if (diff < low_threshold) {
            marble_type_t wrong_type = static_cast<marble_type_t>((expected_marble_type - 1) % NUM_MARBLE_TYPE);
            printf("%d[%d]: (%s) instead of (%s) ____ %d < %d\n", board_index, sensor_index, marble_type_to_string(wrong_type), marble_type_to_string(expected_marble_type), diff, low_threshold);
        }
        if (diff >= high_threshold) {
            marble_type_t wrong_type = static_cast<marble_type_t>((expected_marble_type + 1) % NUM_MARBLE_TYPE);
            printf("%d[%d]: (%s) instead of (%s) ____ %d >= %d\n", board_index, sensor_index, marble_type_to_string(wrong_type), marble_type_to_string(expected_marble_type), diff, high_threshold);
        }
    }
}

void CalibrationTest::_print_marble_placement()
{
    printf("|    0   |    1   |    2   |    3   |    4   |    5   |    6   |    7   |\n");

    for (int i = 0; i < 8; i++)
    {
        printf("|%s", marble_type_to_string(_get_ir_sens_marble_type(i)));
    }
    printf("|\n");

    printf("Measure %d/%d\n", _measure_count + 1, (NUM_IR_SENS_BY_BOARD / 2) * _marbles_for_one_color);
}


marble_type_t CalibrationTest::_get_ir_sens_marble_type(uint8_t ir_sens_channel)
{
    int marble_type;
    uint8_t sensor_line;
    
    if (ir_sens_channel < (NUM_IR_SENS_BY_BOARD / 2))
    {
        sensor_line = ir_sens_channel;
    }
    else
    {
        sensor_line = NUM_IR_SENS_BY_BOARD - 1 - ir_sens_channel;
    }

    uint8_t color_offset = _measure_count / _marbles_for_one_color;

    marble_type = (sensor_line + color_offset) % (NUM_IR_SENS_BY_BOARD / 2);

    if (marble_type >= NUM_MARBLE_TYPE) {
        marble_type = NO_MARBLE;
    }

    return static_cast<marble_type_t>(marble_type);
}
