#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_err.h>
// #include <esp_heap_trace.h>
#include <driver/gpio.h>

#include "IRSensBoards.h"
#include "esp_adc/adc_oneshot.h"
#include "calibration/CalibrationController.h"
#include "BeatsLEDSnake.h"
#include "MasterClock.h"
#include "Sequencer.h"
#include "CalibrationButton.h"

static const char *TAG = "main";



void main_calibrate()
{
    CalibrationController calibration(8, 10, 0);
    // CalibrationController calibration(1, 1, 0);
    while (1)
    {
        calibration.update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


// #define NUM_RECORDS 100
// static heap_trace_record_t trace_record[NUM_RECORDS];

void main_midi_marble_sequencer()
{
    // ESP_ERROR_CHECK( heap_trace_init_standalone(trace_record, NUM_RECORDS) );
    // heap_trace_start(HEAP_TRACE_ALL);

    CalibrationButton &calibration_button = CalibrationButton::instance();
    calibration_button.start_listening_clicks();
    calibration_button.update();
    if (calibration_button.is_down()) {
        ESP_LOGI(TAG, "Calibration button is down at startup, waiting for release before entering calibration mode");
        
        // Wait for button to be released
        while (calibration_button.is_down()) {
            calibration_button.update();
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        
        ESP_LOGI(TAG, "Calibration button released, entering calibration mode");
        main_calibrate();
    }
    else {
        ESP_LOGI(TAG, "Calibration button is up at startup, entering normal mode");
        
        Sequencer sequencer;
        MasterClock master_clock(sequencer);
        vTaskSuspend(NULL); // Suspend main task to let other tasks run
    }

    


    // heap_trace_stop();
    // if (!heap_caps_check_integrity_all(true))
    // {
    //     heap_trace_dump();
    // }

    // while (1)
    // {
    //     // esp_timer_dump(stdout);
    //     // vTaskDelay(pdMS_TO_TICKS(1000));
    // }
}

// void main_test_calibration()
// {
//     CalibrationTest calibration_test(100, 1, 25, 4);
//     while (1)
//     {
//         calibration_test.update();
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// }

void print_board_values(uint32_t *values_off, uint32_t *values_on, uint8_t ir_sens_on_board)
{
    for (int i = 0; i < ir_sens_on_board; i++)
    {
        // printf(">on%d: %d\n", i, values_on[i]);
        // printf(">off%d: %d\n", i, values_off[i]);
        printf(">diff%d: %lu\n", i, values_off[i] - values_on[i]);
    }
}

void main_print_values()
{
    IRSensBoards ir_sens_boards;
    IRSensReader board_reader(&ir_sens_boards);

    uint32_t values_on[NUM_IR_SENS_BY_BOARD];
    uint32_t values_off[NUM_IR_SENS_BY_BOARD];

    // uint64_t read_count = 0;
    while (1)
    {
        board_reader.read_board_values(values_off, values_on, 12, 10);

        print_board_values(values_off, values_on, NUM_IR_SENS_BY_BOARD);
        vTaskDelay(pdMS_TO_TICKS(200));
        // if (read_count > 10) {
        //     statistics(values_off, values_on, &ir_sens_board, 100);
        //     distribution(values_off, values_on, 0, &ir_sens_board, 100);
        // }
    }
}

void main_print_all_boards_first_values()
{
    IRSensBoards ir_sens_boards;
    IRSensReader board_reader(&ir_sens_boards);

    uint32_t value_off, value_on;

    while (1)
    {
        for (size_t i = 0; i < NUM_IR_SENS_BOARDS; i++)
        {
            board_reader.read_sensor_value(&value_off, &value_on, i, 0, 10);
            printf(">diff%d: %lf\n", i, (double) value_off - (double) value_on);
        }
        // board_reader.read_sensor_value(&value_off, &value_on, 4, 0, 10);
        // printf(">diff%d: %lf\n", 4, (double) value_off - (double) value_on);

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void main_print_15_7_values()
{
    IRSensBoards ir_sens_boards;
    IRSensReader board_reader(&ir_sens_boards);

    uint32_t values_on[NUM_IR_SENS_BY_BOARD];
    uint32_t values_off[NUM_IR_SENS_BY_BOARD];

    // uint64_t read_count = 0;
    while (1)
    {
        board_reader.read_board_values(values_off, values_on, 0, 4);

        printf(">ref15: %ld\n", (int32_t)values_off[15] - (int32_t)values_on[15]);
        printf(">7: %ld\n", (int32_t)values_off[7] - (int32_t)values_on[7]);
        vTaskDelay(pdMS_TO_TICKS(500));
        // if (read_count > 10) {
        //     statistics(values_off, values_on, &ir_sens_board, 100);
        //     distribution(values_off, values_on, 0, &ir_sens_board, 100);
        // }
    }
}

void main_test_led_snake()
{
    BeatsLEDSnake led_snake;

    while (1)
    {
        for (size_t i = 0; i < NUM_IR_SENS_BOARDS; i++)
        {
            led_snake.set_eighth_note_index(i * 2);
            led_snake.update();
            vTaskDelay(pdMS_TO_TICKS(600));
        }
    }
}

// void main_calibration_stats_test()
// {
//     uint16_t (*old)[NUM_IR_SENS_BY_BOARD][NUM_MARBLE_TYPE - 1] = marble_types_thresholds_old;
//     uint16_t (*newt)[NUM_IR_SENS_BY_BOARD][NUM_MARBLE_TYPE - 1] = marble_types_thresholds_new;
//     int16_t max_diff = 0;
//     uint64_t total_diff = 0;

//     for (int board = 0; board < NUM_IR_SENS_BOARDS; board++)
//     {
//         for (int sensor = 0; sensor < NUM_IR_SENS_BY_BOARD; sensor++)
//         {
//             for (int type = 0; type < NUM_MARBLE_TYPE - 1; type++)
//             {
//                 int16_t diff = abs(old[board][sensor][type] - newt[board][sensor][type]);

//                 if (diff > max_diff)
//                 {
//                     max_diff = diff;
//                     printf("Max threshold difference between old and new: %d [%u][%u][%u]\n", max_diff, board, sensor, type);
//                 }

//                 total_diff += diff;
//             }
//         }
//     }

//     printf("Max threshold difference between old and new: %d\n", max_diff);
//     printf("Mean threshold difference between old and new: %lld\n", total_diff / (NUM_IR_SENS_BOARDS * NUM_IR_SENS_BY_BOARD * (NUM_MARBLE_TYPE - 1)));

//     int16_t min_interval = 10000;
//     int64_t total_interval = 0;

//     for (int board = 0; board < NUM_IR_SENS_BOARDS; board++)
//     {
//         for (int sensor = 0; sensor < NUM_IR_SENS_BY_BOARD; sensor++)
//         {
//             for (int type = 0; type < NUM_MARBLE_TYPE - 2; type++)
//             {
//                 int16_t interval = newt[board][sensor][type + 1] - newt[board][sensor][type];
//                 total_interval += interval;

//                 if (min_interval > interval)
//                 {
//                     min_interval = interval;
                    
//                     printf("Min interval: %d [%u][%u][%u]\n", min_interval, board, sensor, type);
//                 }
//             }
//         }
//     }

//     printf("Min interval: %d\n", min_interval);
//     printf("Mean interval: %lld\n", total_interval / (NUM_IR_SENS_BOARDS * NUM_IR_SENS_BY_BOARD * (NUM_MARBLE_TYPE - 2)));
// }

extern "C" void app_main()
{
    main_midi_marble_sequencer();

    // main_test_led_snake();

    // main_print_values();

    // main_print_15_7_values();

    // main_print_all_boards_first_values();

    // main_calibrate();

    // main_test_calibration();
}