#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>

#include "IRSensBoards.h"
#include "esp_adc/adc_oneshot.h"
#include "HorizontalCalibration.h"
#include "CalibrationTest.h"
#include "BeatsLEDSnake.h"
#include "MasterClock.h"
#include "Sequencer.h"

void main_midi_marble_sequencer()
{
    Sequencer sequencer;
    MasterClock master_clock(sequencer);
    master_clock.start();

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void main_calibrate()
{
    HorizontalCalibration calibration(8, 10, 25, 4);
    while (1)
    {
        calibration.update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void main_test_calibration()
{
    CalibrationTest calibration_test(100, 1, 25, 4);
    while (1)
    {
        calibration_test.update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

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

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void main_test_led_snake()
{
    BeatsLEDSnake led_snake;

    while (1)
    {
        for (size_t i = 0; i < NUM_IR_SENS_BOARDS; i++)
        {
            led_snake.disable_all_leds();
            led_snake.enable_led(i);
            vTaskDelay(pdMS_TO_TICKS(600));
        }
    }
}

extern "C" void app_main()
{
    esp_log_level_set("*", ESP_LOG_DEBUG);

    main_midi_marble_sequencer();

    // main_test_led_snake();

    // main_print_values();

    // main_print_all_boards_first_values();

    // main_calibrate();

    // main_test_calibration();
}