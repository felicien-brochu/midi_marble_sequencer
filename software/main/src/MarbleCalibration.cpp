#include "MarbleCalibration.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

MarbleCalibration::MarbleCalibration(int marble_type_shifts, int samples_by_test, int ms_between_samples, int multisampling) : _board_reader(&_ir_sens_boards), _push_button(GPIO_NUM_0, true)
{
    _marble_type_shifts = marble_type_shifts;
    _samples_by_test = samples_by_test;
    _ms_between_samples = ms_between_samples;
    _multisampling = multisampling;

    _calibration_state = MARBLE_CALIBRATION_IDLE;
    _first_marble_type = WHITE_MARBLE;
    _measure_count = 0;
    
    _values_on = (int*) malloc(NUM_IR_SENS_BY_BOARD * sizeof(int));
    _values_off = (int*) malloc(NUM_IR_SENS_BY_BOARD * sizeof(int));

    _statistics = (SensorStatistics **) malloc(NUM_IR_SENS_BY_BOARD * (WHITE_MARBLE + 1) * sizeof(SensorStatistics*));

    for (int i = 0; i < NUM_IR_SENS_BY_BOARD; i++) {
        for (int j = 0; j < (WHITE_MARBLE + 1); j++) {
            _statistics[i * (WHITE_MARBLE + 1) + j] = new SensorStatistics(i, (marble_type_t)j, _marble_type_shifts * _samples_by_test);
        }
    }
}

void MarbleCalibration::update()
{
    if (_calibration_state == MARBLE_CALIBRATION_IDLE)
    {
        _idle_state_update();
    }
    else if (_calibration_state == MARBLE_CALIBRATION_WAIT_PLACEMENT)
    {
        _waiting_placement_state_update();
    }
    if (_calibration_state == MARBLE_CALIBRATION_READ)
    {
        _read_state_update();
    }
}

bool MarbleCalibration::is_complete()
{
    return _calibration_state == MARBLE_CALIBRATION_COMPLETE;
}

void MarbleCalibration::_idle_state_update()
{
    _shift_marble_types();
    _print_marble_placement();

    _calibration_state = MARBLE_CALIBRATION_WAIT_PLACEMENT;
}

void MarbleCalibration::_waiting_placement_state_update()
{
    if (!_push_button.has_click_listener()) {
        _push_button.start_listening_clicks();
    }

    _push_button.update();

    if (_push_button.has_click_event_pending()) {
        _push_button.click_event_accounted_for();
        _push_button.stop_listening_clicks();

        _calibration_state = MARBLE_CALIBRATION_READ;

        printf("Measuring...\n");
    }
}

void MarbleCalibration::_read_state_update()
{
    for (int i = 0; i < _samples_by_test; i++) {
        _board_reader.read_board_values(_values_off, _values_on, _multisampling);

        for (int j = 0; j < NUM_IR_SENS_BY_BOARD; j++) {
            _statistics[j * (WHITE_MARBLE + 1) + _get_ir_sens_marble_type(j)]->push_sample(_values_off[j], _values_on[j]);
        }
        vTaskDelay(pdMS_TO_TICKS(_ms_between_samples));
    }
    _measure_count++;

    if (_measure_count >= (WHITE_MARBLE + 1) * _marble_type_shifts)
    {
        _calibration_state = MARBLE_CALIBRATION_COMPLETE;
        _print_statistics();
    }
    else
    {
        _calibration_state = MARBLE_CALIBRATION_IDLE;
    }
}

void MarbleCalibration::_shift_marble_types()
{
    marble_type_t next_marble_type = static_cast<marble_type_t>((_first_marble_type + 1) % (WHITE_MARBLE + 1));
    _first_marble_type = next_marble_type;
}

void MarbleCalibration::_print_marble_placement()
{
    printf("|    0   |    1   |    2   |    3   |    4   |    5   |    6   |    7   |\n");

    for (int i = 0; i < 8; i++)
    {
        printf("|%s", marble_type_to_string(_get_ir_sens_marble_type(i)));
    }
    printf("|\n");

    for (int i = 15; i >= 8; i--)
    {
        printf("|%s", marble_type_to_string(_get_ir_sens_marble_type(i)));
    }
    printf("|\n");

    printf("|   15   |   14   |   13   |   12   |   11   |   10   |    9   |    8   |\n");
    printf("\n");

    printf("Measure %d/%d\n", _measure_count + 1, (WHITE_MARBLE + 1) * _marble_type_shifts);
}

void MarbleCalibration::_print_marble_intervals_for_sensor(uint8_t ir_sens_channel)
{
    printf("Sens%2d:", ir_sens_channel);
    for (int j = 0; j <= WHITE_MARBLE; j++)
    {
        SensorStatistics *statsJ = _statistics[ir_sens_channel * (WHITE_MARBLE + 1) + j];
        printf(" %5.0f <%s> %5.0f |", statsJ->diff_min, marble_type_to_string(statsJ->marble_type), statsJ->diff_max);

        if (j < WHITE_MARBLE)
        {
            SensorStatistics *statsJ1 = _statistics[ir_sens_channel * (WHITE_MARBLE + 1) + j + 1];
            double deltaMaxJminJ1 = statsJ1->diff_min - statsJ->diff_max;
            double middleMaxJminJ1 = (statsJ1->diff_min + statsJ->diff_max) / 2;
            double loss = 0;

            if (deltaMaxJminJ1 < 0)
            {
                // printf(" %5.0f |", deltaMaxJminJ1);
                loss = statsJ->compute_loss_to_next_marble_type(statsJ1, &middleMaxJminJ1);
            }
            printf(" %5.0f(%2.3f%%) |", middleMaxJminJ1, loss * 100);
        }
    }
    printf("\n");
}

void MarbleCalibration::_print_statistics()
{
    // for (int i = 0; i < NUM_IR_SENS_BY_BOARD; i++)
    // {
    //     printf("Sens%d:\n", i);
    //     for (int j = 0; j < (WHITE_MARBLE + 1); j++)
    //     {
    //         SensorStatistics *stats = _statistics[i * (WHITE_MARBLE + 1) + j];
    //         stats->compute_statistics();
    //         printf("%s\n", marble_type_to_string(stats->marble_type));
    //         printf("  mean:    %4.0f\n", stats->diff_mean);
    //         printf("  median:  %4.0f\n", stats->diff_median);
    //         printf("  min:     %4.0f\n", stats->diff_min);
    //         printf("  min999:  %4.0f\n", stats->min999);
    //         printf("  min9999: %4.0f\n", stats->min9999);
    //         printf("  max:     %4.0f\n", stats->diff_max);
    //         printf("  max999:  %4.0f\n", stats->max999);
    //         printf("  max9999: %4.0f\n", stats->max9999);
    //         printf("\n");
    //     }

    //     vTaskDelay(pdMS_TO_TICKS(10));
    // }

    // printf("\n");
    // printf("\n");

    for (int i = 0; i < NUM_IR_SENS_BY_BOARD / 2; i++)
    {
        _print_marble_intervals_for_sensor(i);
        _print_marble_intervals_for_sensor(NUM_IR_SENS_BY_BOARD - 1 - i);
    }
}

marble_type_t MarbleCalibration::_get_ir_sens_marble_type(uint8_t ir_sens_channel)
{
    int marble_type;
    
    if (ir_sens_channel < 8)
    {
        marble_type = (_first_marble_type + ir_sens_channel) % (WHITE_MARBLE + 1);
    }
    else
    {
        marble_type = (_first_marble_type + (15 - ir_sens_channel)) % (WHITE_MARBLE + 1);
    }

    return static_cast<marble_type_t>(marble_type);
}
