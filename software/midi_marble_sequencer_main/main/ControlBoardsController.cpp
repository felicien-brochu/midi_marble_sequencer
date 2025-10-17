#include "ControlBoardsController.h"

#include "crc.h"

#include <cstring>
#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "ControlBoardsController";

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
            .disable_ack_check = false
        }
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(_i2c_bus_handle, &controls_main_device_config, &_controls_main_device_handle));
}

void ControlBoardsController::start_control_boards_task()
{
    TaskHandle_t control_boards_task_handle;
    xTaskCreate(_control_boards_controller_task, "ControlBoardsController", 3500, this, 1, &control_boards_task_handle);
}

void ControlBoardsController::main_task()
{
    esp_err_t err;
    while (true)
    {
        // while (true)
        // {
        //     err = i2c_master_probe(_i2c_bus_handle, I2C_CONFIG_CONTROLS_MAIN_ADDR, -1);
        //     if (err == ESP_OK)
        //     {
        //         break;
        //     }
        // }

        while (true)
        {
            controls_main_display_t controls_main_display = _sequencer.get_controls_main_display();
            memcpy(_write_buffer, &controls_main_display, sizeof(controls_main_display_t));

            compute_crc16(_write_buffer, sizeof(controls_main_display_t));

            err = i2c_master_transmit_receive(_controls_main_device_handle, _write_buffer, sizeof(controls_main_display_t), _read_buffer, sizeof(controls_main_value_t), -1);

            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "transmit_receive err : %x", err);
                break;
            }

            if (!check_crc16(_read_buffer, sizeof(controls_main_value_t)))
            {
                ESP_LOGE(TAG, "CRC check FAILED!");
                ESP_ERROR_CHECK(i2c_master_bus_reset(_i2c_bus_handle));
                break;
            }
            
            controls_main_value_t controls_main_value;
            memcpy(&controls_main_value, _read_buffer, sizeof(controls_main_value_t));

            _sequencer.handle_controls_main_event(controls_main_value);
            
            vTaskDelay(pdMS_TO_TICKS(I2C_MASTER_TIMER_PERIOD / 1000));
        }
    }
}
