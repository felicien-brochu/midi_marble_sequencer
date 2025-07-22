#include "HorizontalCalibration.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

ColorStatistics::ColorStatistics()
{
    min = 1 << 15;
    max = 0;
    sum = 0;
    nb_samples = 0;
    mean = 0;
}

void ColorStatistics::push_sample(int value_off, int value_on)
{
    int diff = value_off - value_on;

    if (min > diff)
    {
        min = diff;
    }

    if (max < diff)
    {
        max = diff;
    }

    sum += diff;

    nb_samples++;
    mean = sum / nb_samples;
}


SensorStatistics::SensorStatistics()
{
}

void SensorStatistics::push_sample(marble_type_t color, int value_off, int value_on)
{
    color_statistics[color].push_sample(value_off, value_on);
}

void SensorStatistics::compute_thresholds(uint16_t *thresholds)
{
    for (size_t i = 0; i < NUM_MARBLE_TYPE - 1; i++)
    {
        uint16_t up_var = color_statistics[i].max - color_statistics[i].mean;
        uint16_t down_var = color_statistics[i + 1].mean - color_statistics[i + 1].min;

        float k = ((float) up_var) / (up_var + down_var);
        uint16_t threshold = (uint16_t) (color_statistics[i].max + k * (color_statistics[i + 1].min - color_statistics[i].max));

        // First and last threshold are not scaled by variance because first and last marble types have truncated (saturated) measurements
        // Instead we choose the middle between low max et high min.
        if (i == 0 || i == NUM_MARBLE_TYPE - 2)
        {
            threshold = color_statistics[i].max + ((color_statistics[i + 1].min - color_statistics[i].max) / 2);
        }
        thresholds[i] = threshold;
    }
}


HorizontalCalibration::HorizontalCalibration(int marbles_for_one_color, int samples_by_test, int ms_between_samples, int multisampling) : _board_reader(&_ir_sens_boards), _push_button(GPIO_NUM_3, false)
{
    _marbles_for_one_color = marbles_for_one_color;
    _samples_by_test = samples_by_test;
    _ms_between_samples = ms_between_samples;
    _multisampling = multisampling;

    _calibration_state = HORIZONTAL_CALIBRATION_IDLE;
    _measure_count = 0;

    uint16_t num_sensor_stats = NUM_IR_SENS_BOARDS * NUM_IR_SENS_BY_BOARD;
    _statistics = (SensorStatistics **) malloc(num_sensor_stats * sizeof(SensorStatistics *));

    for (size_t i = 0; i < num_sensor_stats; i++)
    {
        _statistics[i] = new SensorStatistics();
    }
    
    
    _values_on = (uint32_t*) malloc(NUM_IR_SENS_BY_BOARD * sizeof(uint32_t));
    _values_off = (uint32_t*) malloc(NUM_IR_SENS_BY_BOARD * sizeof(uint32_t));

    vTaskDelay(pdMS_TO_TICKS(3000));
}

void HorizontalCalibration::update()
{
    if (_calibration_state == HORIZONTAL_CALIBRATION_IDLE)
    {
        _idle_state_update();
    }
    else if (_calibration_state == HORIZONTAL_CALIBRATION_WAIT_PLACEMENT)
    {
        _waiting_placement_state_update();
    }
    if (_calibration_state == HORIZONTAL_CALIBRATION_READ)
    {
        _read_state_update();
    }
}

bool HorizontalCalibration::is_complete()
{
    return _calibration_state == HORIZONTAL_CALIBRATION_COMPLETE;
}

void HorizontalCalibration::_idle_state_update()
{
    _print_marble_placement();

    _calibration_state = HORIZONTAL_CALIBRATION_WAIT_PLACEMENT;
}

void HorizontalCalibration::_waiting_placement_state_update()
{
    if (!_push_button.has_click_listener()) {
        _push_button.start_listening_clicks();
    }

    _push_button.update();

    if (_push_button.has_click_event_pending()) {
        _push_button.click_event_accounted_for();
        _push_button.stop_listening_clicks();

        _calibration_state = HORIZONTAL_CALIBRATION_READ;

        printf("Measuring...\n");
    }
}

void HorizontalCalibration::_read_state_update()
{
    for (size_t board_index = 0; board_index < NUM_IR_SENS_BOARDS; board_index++)
    {
        for (int i = 0; i < _samples_by_test; i++) {
            _board_reader.read_board_values(_values_off, _values_on, board_index, _multisampling);

            for (int j = 0; j < NUM_IR_SENS_BY_BOARD; j++) {
                _statistics[board_index * NUM_IR_SENS_BY_BOARD + j]->push_sample(_get_ir_sens_marble_type(j), _values_off[j], _values_on[j]);
            }
            vTaskDelay(pdMS_TO_TICKS(_ms_between_samples));
        }
    }
        
    
    _measure_count++;

    if (_measure_count >= (NUM_IR_SENS_BY_BOARD / 2) * _marbles_for_one_color)
    {
        _calibration_state = HORIZONTAL_CALIBRATION_COMPLETE;
        _print_statistics();
        _print_csv_data();
        _print_c_array_thresholds();
        // _print_c_array_means();
        
        fflush(stdout);
    }
    else
    {
        _calibration_state = HORIZONTAL_CALIBRATION_IDLE;
    }
}

void HorizontalCalibration::_print_marble_placement()
{
    printf("|    0   |    1   |    2   |    3   |    4   |    5   |    6   |    7   |\n");

    for (int i = 0; i < 8; i++)
    {
        printf("|%s", marble_type_to_string(_get_ir_sens_marble_type(i)));
    }
    printf("|\n");

    printf("Measure %d/%d\n", _measure_count + 1, (NUM_IR_SENS_BY_BOARD / 2) * _marbles_for_one_color);
}

void HorizontalCalibration::_print_marble_intervals_for_sensor(uint8_t board_index, uint8_t ir_sens_channel)
{
    SensorStatistics *sensor_stats = _statistics[board_index * NUM_IR_SENS_BY_BOARD + ir_sens_channel];
    printf("%2dSens%2d:", board_index, ir_sens_channel);
    uint16_t thresholds[NUM_MARBLE_TYPE - 1];
    sensor_stats->compute_thresholds(thresholds);

    for (int i = 0; i < NUM_MARBLE_TYPE; i++)
    {
        ColorStatistics color_stats = sensor_stats->color_statistics[i];
        printf(" %4d <%s> %4d |", color_stats.min, marble_type_to_string(static_cast<marble_type_t>(i)), color_stats.max);

        if (i < NUM_MARBLE_TYPE - 1)
        {
            printf(" %4d |", thresholds[i]);
        }
    }
    printf("\n");
}

void HorizontalCalibration::_print_statistics()
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

void HorizontalCalibration::_print_csv_data()
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
                printf("%d;%d;\"%s\";%d;%d;%d;", board_index, ir_sens_channel, marble_type_to_string(static_cast<marble_type_t>(marble_type)), color_stats.min, color_stats.max, color_stats.mean);

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

void HorizontalCalibration::_print_c_array_thresholds()
{
    for (size_t board_index = 0; board_index < NUM_IR_SENS_BOARDS; board_index++)
    {
        for (int ir_sens_channel = 0; ir_sens_channel < NUM_IR_SENS_BY_BOARD; ir_sens_channel++)
        {
            SensorStatistics *sensor_stats = _statistics[board_index * NUM_IR_SENS_BY_BOARD + ir_sens_channel];
            uint16_t thresholds[NUM_MARBLE_TYPE - 1];
            sensor_stats->compute_thresholds(thresholds);
            printf("{");
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
            printf("\n");
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void HorizontalCalibration::_print_c_array_means()
{
    for (size_t board_index = 0; board_index < NUM_IR_SENS_BOARDS; board_index++)
    {
        for (int ir_sens_channel = 0; ir_sens_channel < NUM_IR_SENS_BY_BOARD; ir_sens_channel++)
        {
            SensorStatistics *sensor_stats = _statistics[board_index * NUM_IR_SENS_BY_BOARD + ir_sens_channel];
            printf("{");
            for (int j = 0; j < NUM_MARBLE_TYPE; j++)
            {
                printf("%4d", sensor_stats->color_statistics[j].mean);

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

marble_type_t HorizontalCalibration::_get_ir_sens_marble_type(uint8_t ir_sens_channel)
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
