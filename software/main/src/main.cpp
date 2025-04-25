#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include "IRSensBoards.h"
#include "IRSensReader.h"
#include "analytics.h"
#include "MarbleCalibration.h"
#include "MarbleChangeDetector.h"
#include "BeatsLEDSnake.h"
#include "i2c_master_test.h"

void main_print_values()
{
    IRSensBoards ir_sens_boards;
    IRSensReader board_reader(&ir_sens_boards);

    uint8_t board_index = 1;

    gpio_reset_pin(GPIO_NUM_39);
    gpio_set_direction(GPIO_NUM_39, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_39, board_index & 1);

    gpio_reset_pin(GPIO_NUM_40);
    gpio_set_direction(GPIO_NUM_40, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_40, board_index & 2);

    gpio_reset_pin(GPIO_NUM_41);
    gpio_set_direction(GPIO_NUM_41, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_41, board_index & 4);

    gpio_reset_pin(GPIO_NUM_42);
    gpio_set_direction(GPIO_NUM_42, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_42, board_index & 8);

    int values_on[NUM_IR_SENS_BY_BOARD];
    int values_off[NUM_IR_SENS_BY_BOARD];

    uint64_t read_count = 0;
    while (1)
    {
        board_reader.read_board_values(values_off, values_on, board_index, 10);

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

    int value_off, value_on;

    while (1)
    {
        for (size_t i = 0; i < NUM_IR_SENS_BOARDS; i++)
        {
            board_reader.read_sensor_value(&value_off, &value_on, i, 0, 10);
            printf(">diff%d: %d\n", i, value_off - value_on);
        }

        vTaskDelay(pdMS_TO_TICKS(200));
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

        main_print_all_boards_first_values();

        // main_calibrate();

        // main_print_marble_changes();

        // main_test_led_snake();

        // main_test_i2c_master();
    }

#ifdef __cplusplus
}
#endif