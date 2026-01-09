#include "CalibrationController.h"
#include "CalibrationButton.h"
#include "CalibrationStorage.h"
#include "BoardCalibrationPhase.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

const static char *TAG = "CalibrationController";

CalibrationController::CalibrationController(int marbles_for_one_color, int samples_by_test, int ms_between_samples) : _board_reader(&_ir_sens_boards), _push_button(CalibrationButton::instance()), _control_boards_controller(_display)
{
    _marbles_for_one_color = marbles_for_one_color;
    _samples_by_test = samples_by_test;
    _ms_between_samples = ms_between_samples;

    _calibration_state = CALIBRATION_BOARD_PHASE;
    _measure_count = 0;
    
    // Initialize board calibration phase
    _board_calibration_phase = new BoardCalibrationPhase(&_ir_sens_boards, &_board_reader, _push_button, &_display, &_beats_led_snake);

    uint16_t num_sensor_stats = NUM_IR_SENS_BOARDS * NUM_IR_SENS_BY_BOARD;
    _statistics = (SensorStatistics **) malloc(num_sensor_stats * sizeof(SensorStatistics *));

    for (size_t i = 0; i < num_sensor_stats; i++)
    {
        _statistics[i] = new SensorStatistics();
    }
    
    
    _values_on = (uint32_t*) malloc(NUM_IR_SENS_BY_BOARD * sizeof(uint32_t));
    _values_off = (uint32_t*) malloc(NUM_IR_SENS_BY_BOARD * sizeof(uint32_t));

    _control_boards_controller.start_main_task();
}

void CalibrationController::update()
{
    if (_calibration_state == CALIBRATION_BOARD_PHASE)
    {
        _update_board_calibration_phase();
    }
    else if (_calibration_state == CALIBRATION_IDLE)
    {
        _update_idle_state();
    }
    else if (_calibration_state == CALIBRATION_WAIT_PLACEMENT)
    {
        _update_waiting_placement_state();
    }
    if (_calibration_state == CALIBRATION_READ)
    {
        _update_read_state();
    }
}

void CalibrationController::_update_board_calibration_phase()
{
    _board_calibration_phase->update();
    if (_board_calibration_phase->is_complete())
    {
        delete _board_calibration_phase;
        _board_calibration_phase = nullptr;
        _calibration_state = CALIBRATION_IDLE;
    }
}

bool CalibrationController::is_complete()
{
    return _calibration_state == CALIBRATION_COMPLETE;
}

void CalibrationController::_update_idle_state()
{
    _print_marble_placement();

    _calibration_state = CALIBRATION_WAIT_PLACEMENT;
}

void CalibrationController::_update_waiting_placement_state()
{
    if (!_push_button.has_click_listener()) {
        _push_button.start_listening_clicks();
    }

    _push_button.update();

    if (_push_button.has_click_event_pending()) {
        _push_button.click_event_accounted_for();
        _push_button.stop_listening_clicks();

        _calibration_state = CALIBRATION_READ;

        printf("Measuring...\n");

        _display.hide_marble_placement();
    }
}

void CalibrationController::_update_read_state()
{
    for (size_t board_index = 0; board_index < NUM_IR_SENS_BOARDS; board_index++)
    {
        for (int i = 0; i < _samples_by_test; i++) {
            _board_reader.read_board_values(_values_off, _values_on, board_index, IR_SENSOR_MULTISAMPLING);

            for (int j = 0; j < NUM_IR_SENS_BY_BOARD; j++) {
                _statistics[board_index * NUM_IR_SENS_BY_BOARD + j]->push_sample(_get_ir_sens_marble_type(j), _values_off[j], _values_on[j]);
            }

            if (_ms_between_samples >= 10)
            {
                vTaskDelay(pdMS_TO_TICKS(_ms_between_samples));
            }
        }
    }
        
    
    _measure_count++;

    if (_measure_count >= _get_total_measure_count())
    {
        _on_measure_complete();
    }
    else
    {
        _calibration_state = CALIBRATION_IDLE;
    }
}

void CalibrationController::_on_measure_complete()
{
    _calibration_state = CALIBRATION_COMPLETE;
    // _print_statistics();
    // _print_c_array_means();
    _display.show_completion_rate(1./8.);

    _print_csv_data();
    fflush(stdout);
    _display.show_completion_rate(3./8.);

    _print_c_array_thresholds();
    fflush(stdout);
    _display.show_completion_rate(5./8.);

    bool confirmed = _ask_confirmation_to_save();
    if (!confirmed) {
        return;
    }
    _save_calibration_data_to_nvs();
    ESP_LOGI(TAG, "Calibration data saved successfuly.");
    _display.show_completion_rate(8./8.);
}

void CalibrationController::_sensor_statistics_to_thresholds(uint16_t (*out_thresholds)[NUM_IR_SENS_BY_BOARD][NUM_MARBLE_TYPE - 1])
{
    for (size_t board_index = 0; board_index < NUM_IR_SENS_BOARDS; board_index++)
    {
        for (int ir_sens_channel = 0; ir_sens_channel < NUM_IR_SENS_BY_BOARD; ir_sens_channel++)
        {
            SensorStatistics *sensor_stats = _statistics[board_index * NUM_IR_SENS_BY_BOARD + ir_sens_channel];
            sensor_stats->compute_thresholds(out_thresholds[board_index][ir_sens_channel]);
        }
    }
}

void CalibrationController::_save_calibration_data_to_nvs()
{
    CalibrationStorage storage;
    uint16_t (*marble_types_thresholds)[NUM_IR_SENS_BY_BOARD][NUM_MARBLE_TYPE - 1] = (uint16_t (*)[NUM_IR_SENS_BY_BOARD][NUM_MARBLE_TYPE - 1]) malloc(sizeof(uint16_t[NUM_IR_SENS_BOARDS][NUM_IR_SENS_BY_BOARD][NUM_MARBLE_TYPE - 1]));

    _sensor_statistics_to_thresholds(marble_types_thresholds);

    esp_err_t err = storage.save_calibration_data(marble_types_thresholds);

    if (err == ESP_OK)
    {
        printf("Calibration data saved to NVS.\n");
    }
}

bool CalibrationController::_ask_confirmation_to_save()
{
    printf("Save calibration data? Press button for 5 seconds to confirm.\n");

    // Wait for button down
    while (true)
    {
        _push_button.update();

        if (_push_button.is_down()) {
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Wait for 5 seconds of button hold
    uint32_t hold_start_time = xTaskGetTickCount();
    while (true)
    {
        _push_button.update();

        if (_push_button.is_up()) {
            printf("Calibration data NOT saved.\n");
            return false;
        }

        TickType_t current_time = xTaskGetTickCount();
        if (current_time - hold_start_time >= pdMS_TO_TICKS(5000)) {
            return true;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    return false;
}

int CalibrationController::_get_total_measure_count()
{
    return (SEQUENCER_TRACKS_NUM / _get_num_marble_types_groups()) * _marbles_for_one_color;
}

int CalibrationController::_get_num_marble_types_groups()
{
    return (SEQUENCER_TRACKS_NUM / NUM_MARBLE_TYPE);
}

void CalibrationController::_print_marble_placement()
{
    _display.hide_marble_placement();
    printf("|    0   |    1   |    2   |    3   |    4   |    5   |    6   |    7   |\n");

    for (int i = 0; i < 8; i++)
    {
        marble_type_t marble_type = _get_ir_sens_marble_type(i);
        printf("|%s", marble_type_to_string(marble_type));

        if (marble_type != NO_MARBLE) {
            _display.set_led_enabled(0, i, true);
        }
    }
    printf("|\n");

    printf("Measure %d/%d\n", _measure_count + 1, _get_total_measure_count());

    _display.show_completion_rate((float)(_measure_count + 1) / (float) _get_total_measure_count());
}

void CalibrationController::_print_marble_intervals_for_sensor(uint8_t board_index, uint8_t ir_sens_channel)
{
    SensorStatistics *sensor_stats = _statistics[board_index * NUM_IR_SENS_BY_BOARD + ir_sens_channel];
    printf("%2dSens%2d:", board_index, ir_sens_channel);
    uint16_t thresholds[NUM_MARBLE_TYPE - 1];
    sensor_stats->compute_thresholds(thresholds);

    for (int i = 0; i < NUM_MARBLE_TYPE; i++)
    {
        ColorStatistics color_stats = sensor_stats->color_statistics[i];
        printf(" %4ld <%s> %4ld |", color_stats.min, marble_type_to_string(static_cast<marble_type_t>(i)), color_stats.max);

        if (i < NUM_MARBLE_TYPE - 1)
        {
            printf(" %4d |", thresholds[i]);
        }
    }
    printf("\n");
}

void CalibrationController::_print_statistics()
{
    for (size_t board_index = 0; board_index < NUM_IR_SENS_BOARDS; board_index++)
    {
        for (int i = 0; i < NUM_IR_SENS_BY_BOARD / 2; i++)
        {
            _print_marble_intervals_for_sensor(board_index, i);
            _print_marble_intervals_for_sensor(board_index, NUM_IR_SENS_BY_BOARD - 1 - i);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void CalibrationController::_print_csv_data()
{
    printf("board;sensor;marble_color;min_value;max_value;mean;low_threshold;high_threshold\n");
    for (size_t board_index = 0; board_index < NUM_IR_SENS_BOARDS; board_index++)
    {
        for (int ir_sens_channel = 0; ir_sens_channel < NUM_IR_SENS_BY_BOARD; ir_sens_channel++)
        {
            SensorStatistics *sensor_stats = _statistics[board_index * NUM_IR_SENS_BY_BOARD + ir_sens_channel];
            uint16_t thresholds[NUM_MARBLE_TYPE - 1];
            sensor_stats->compute_thresholds(thresholds);
            uint16_t prev_threshold = 0;

            for (int marble_type = 0; marble_type < NUM_MARBLE_TYPE; marble_type++)
            {
                ColorStatistics color_stats = sensor_stats->color_statistics[marble_type];
                printf("%d;%d;\"%s\";%ld;%ld;%ld;", board_index, ir_sens_channel, marble_type_to_string(static_cast<marble_type_t>(marble_type)), color_stats.min, color_stats.max, color_stats.mean);

                printf("%d;", prev_threshold);

                if (marble_type < NUM_MARBLE_TYPE - 1)
                {
                    uint16_t current_threshold = thresholds[marble_type];

                    printf("%d", current_threshold);
                    prev_threshold = current_threshold;
                }
                else
                {
                    printf("%d", 4095);
                }
                printf("\n");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void CalibrationController::_print_c_array_thresholds()
{
    printf("const uint16_t marble_types_thresholds[NUM_IR_SENS_BOARDS][NUM_IR_SENS_BY_BOARD][NUM_MARBLE_TYPE - 1] = {\n");

    for (size_t board_index = 0; board_index < NUM_IR_SENS_BOARDS; board_index++)
    {
        printf("\t{\n");
        for (int ir_sens_channel = 0; ir_sens_channel < NUM_IR_SENS_BY_BOARD; ir_sens_channel++)
        {
            SensorStatistics *sensor_stats = _statistics[board_index * NUM_IR_SENS_BY_BOARD + ir_sens_channel];
            uint16_t thresholds[NUM_MARBLE_TYPE - 1];
            sensor_stats->compute_thresholds(thresholds);
            printf("\t\t{");
            for (int j = 0; j < NUM_MARBLE_TYPE - 1; j++)
            {
                printf("%4d", thresholds[j]);

                // Not last threshold
                if (j < NUM_MARBLE_TYPE - 2)
                {
                    printf(", ");
                }
            }

            printf("}");

            // Not last sensor
            if (ir_sens_channel < NUM_IR_SENS_BY_BOARD - 1)
            {
                printf(",");
            }
            else
            {
                printf("\n\t}");

                if (board_index < NUM_IR_SENS_BOARDS - 1)
                {
                    printf(",");
                }
            }
            printf("\n");
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
        
    printf("};\n");
}

void CalibrationController::_print_c_array_means()
{
    for (size_t board_index = 0; board_index < NUM_IR_SENS_BOARDS; board_index++)
    {
        for (int ir_sens_channel = 0; ir_sens_channel < NUM_IR_SENS_BY_BOARD; ir_sens_channel++)
        {
            SensorStatistics *sensor_stats = _statistics[board_index * NUM_IR_SENS_BY_BOARD + ir_sens_channel];
            printf("{");
            for (int j = 0; j < NUM_MARBLE_TYPE; j++)
            {
                printf("%4ld", sensor_stats->color_statistics[j].mean);

                // Not last mean
                if (j < NUM_MARBLE_TYPE - 1)
                {
                    printf(", ");
                }
            }

            printf("}");

            // Not last sensor
            if (ir_sens_channel < NUM_IR_SENS_BY_BOARD - 1)
            {
                printf(",");
            }
            printf("\n");
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

marble_type_t CalibrationController::_get_ir_sens_marble_type(uint8_t ir_sens_channel)
{
    int marble_type;
    uint8_t sensor_line;
    
    if (ir_sens_channel < SEQUENCER_TRACKS_NUM)
    {
        sensor_line = ir_sens_channel;
    }
    else
    {
        sensor_line = NUM_IR_SENS_BY_BOARD - 1 - ir_sens_channel;
    }

    uint8_t color_offset = _measure_count / _marbles_for_one_color;

    marble_type = (sensor_line + color_offset) % SEQUENCER_TRACKS_NUM;
    if (_get_num_marble_types_groups() <= 1 && marble_type >= NUM_MARBLE_TYPE) {
        marble_type = NO_MARBLE;
    }
    else {
        marble_type = marble_type % NUM_MARBLE_TYPE;
    }

    return static_cast<marble_type_t>(marble_type);
}
