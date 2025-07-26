#include "ControlBoardsController.h"

#include <cstring>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>

static void _control_boards_controller_task(void *control_boards_controller_arg)
{
    ControlBoardsController *control_boards_controller = (ControlBoardsController *)control_boards_controller_arg;
    control_boards_controller->main_task();
}

ControlBoardsController::ControlBoardsController(Sequencer &sequencer) : _sequencer(sequencer)
{
    _write_buffer = (uint8_t *) malloc(sizeof(controls_main_display_t));
    _read_buffer = (uint8_t *) malloc(sizeof(controls_main_value_t));

    i2c_master_bus_config_t i2c_master_config = {
        .i2c_port = I2C_CONFIG_PORT,
        .sda_io_num = I2C_MASTER_SDA_GPIO,
        .scl_io_num = I2C_MASTER_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {
            .enable_internal_pullup = false,
        }
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_master_config, &_i2c_bus_handle));

    i2c_device_config_t controls_main_device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = I2C_CONFIG_CONTROLS_MAIN_ADDR,
        .scl_speed_hz = I2C_CONFIG_CLOCK_SPEED_HZ,
        .scl_wait_us = 0,
        .flags = {
            .disable_ack_check = true
        }
    };

    ESP_ERROR_CHECK(i2c_master_probe(_i2c_bus_handle, I2C_CONFIG_CONTROLS_MAIN_ADDR, -1));
    ESP_ERROR_CHECK(i2c_master_bus_add_device(_i2c_bus_handle, &controls_main_device_config, &_controls_main_device_handle));
}

void ControlBoardsController::start_control_boards_task()
{
    // const esp_timer_create_args_t periodic_timer_args = {
    //     .callback = &_control_boards_controller_task,
    //     .arg = this,
    //     .dispatch_method = ESP_TIMER_TASK,
    //     .name = "ControlBoardsController"};

    // esp_timer_handle_t periodic_timer;
    // ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    // ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, I2C_MASTER_TIMER_PERIOD));
    TaskHandle_t control_boards_task_handle;
    xTaskCreate(_control_boards_controller_task, "ControlBoardsController", 10000, this, 3, &control_boards_task_handle);
}

void ControlBoardsController::main_task()
{
    while(true)
    {
        controls_main_display_t controls_main_display = _sequencer.get_controls_main_display();
        memcpy(_write_buffer, &controls_main_display, sizeof(controls_main_display_t));

        // printf("####TRANSMIT play_led: %d\n", controls_main_display.play_pause_led_enabled);

        ESP_ERROR_CHECK(i2c_master_transmit_receive(_controls_main_device_handle, _write_buffer, sizeof(controls_main_display_t), _read_buffer, sizeof(controls_main_value_t), -1));

        
        controls_main_value_t controls_main_value;
        memcpy(&controls_main_value, _read_buffer, sizeof(controls_main_value_t));

        _sequencer.handle_controls_main_event(controls_main_value);

        // printf(".");
        // printf("####RECEIVE play_led: %d\n", controls_main_value.play_pause_switch_pushed);
        
        vTaskDelay(pdMS_TO_TICKS(I2C_MASTER_TIMER_PERIOD / 1000));
    }
}
