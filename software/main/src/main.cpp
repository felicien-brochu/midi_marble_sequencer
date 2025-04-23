#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include "IRSensBoard.h"
#include "IRSensBoardReaderOneShot.h"
#include "analytics.h"
#include "MarbleCalibration.h"
#include "MarbleChangeDetector.h"
#include "BeatsLEDSnake.h"
#include "i2c_master_test.h"

void main_print_values()
{
    IRSensBoard ir_sens_board;
    IRSensBoardReaderOneShot board_reader(&ir_sens_board);

    int values_on[ir_sens_board.ir_sens_on_board];
    int values_off[ir_sens_board.ir_sens_on_board];

    uint64_t read_count = 0;
    while (1)
    {
        board_reader.read_values(values_off, values_on, 10);
        read_count++;

        print_values(values_off, values_on, ir_sens_board.ir_sens_on_board);
        vTaskDelay(pdMS_TO_TICKS(200));
        // if (read_count > 10) {
        //     statistics(values_off, values_on, &ir_sens_board, 100);
        //     distribution(values_off, values_on, 0, &ir_sens_board, 100);
        // }
    }
}

void main_calibrate()
{
    MarbleCalibration calibration(10, 10, 50, 4);

    while (1)
    {
        calibration.update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void main_print_marble_changes()
{
    MarbleChangeDetector marble_change_detector;

    while (1)
    {
        marble_change_detector.update();
        vTaskDelay(pdMS_TO_TICKS(40));
    }
}

void main_test_led_snake()
{
    BeatsLEDSnake led_snake;

    while (1)
    {
        led_snake.disable_all_leds();
        led_snake.enable_led(0);
        vTaskDelay(pdMS_TO_TICKS(650));
        
        led_snake.disable_all_leds();
        led_snake.enable_led(1);
        vTaskDelay(pdMS_TO_TICKS(650));

        led_snake.disable_all_leds();
        led_snake.enable_led(2);
        vTaskDelay(pdMS_TO_TICKS(650));

        led_snake.disable_all_leds();
        led_snake.enable_led(3);
        vTaskDelay(pdMS_TO_TICKS(650));

        led_snake.disable_all_leds();
        led_snake.enable_led(4);
        vTaskDelay(pdMS_TO_TICKS(650));
    }
}

#ifdef __cplusplus
    extern "C"
{
#endif

    void app_main()
    {
        esp_log_level_set("*", ESP_LOG_INFO);

        // main_print_values();

        // main_calibrate();

        // main_print_marble_changes();

        main_test_led_snake();

        // main_test_i2c_master();
    }

#ifdef __cplusplus
}
#endif