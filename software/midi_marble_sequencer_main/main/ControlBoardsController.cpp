#include "ControlBoardsController.h"

#include "crc.h"

#include <cstring>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "ControlBoardsController";

static void _main_task_cb(void *control_boards_controller_arg)
{
    ControlBoardsController *control_boards_controller = (ControlBoardsController *)control_boards_controller_arg;
    control_boards_controller->main_task();
}

static void _display_only_task_cb(void *control_boards_controller_arg)
{
    ControlBoardsController *control_boards_controller = (ControlBoardsController *)control_boards_controller_arg;
    control_boards_controller->display_only_task();
}

ControlBoardsController::ControlBoardsController(IControlBoardsListener &listener) : _listener(listener)
{
    i2c_master_bus_config_t i2c_master_config = {
        .i2c_port = I2C_CONFIG_PORT,
        .sda_io_num = I2C_MASTER_SDA_GPIO,
        .scl_io_num = I2C_MASTER_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        // .trans_queue_depth = 128,
        .flags = {
            .enable_internal_pullup = false,
            .allow_pd = false,
        }
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_master_config, &_i2c_bus_handle));

    _init_main_controls_i2c();
    _init_pages_controls_i2c();
}

void ControlBoardsController::_init_main_controls_i2c()
{
    _main_write_buffer = (uint8_t *) malloc(sizeof(controls_main_display_t));

    i2c_device_config_t controls_main_device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = I2C_CONFIG_CONTROLS_MAIN_ADDR,
        .scl_speed_hz = I2C_CONFIG_CLOCK_SPEED_HZ,
        .scl_wait_us = 5000,
        .flags = {
            .disable_ack_check = true
        }
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(_i2c_bus_handle, &controls_main_device_config, &_controls_main_device_handle));
}

void ControlBoardsController::_init_pages_controls_i2c()
{
    _pages_write_buffer = (uint8_t *) malloc(sizeof(controls_pages_display_t));

    i2c_device_config_t controls_pages_device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = I2C_CONFIG_CONTROLS_PAGES_ADDR,
        .scl_speed_hz = I2C_CONFIG_CLOCK_SPEED_HZ,
        .scl_wait_us = 0,
        .flags = {
            .disable_ack_check = true
        }
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(_i2c_bus_handle, &controls_pages_device_config, &_controls_pages_device_handle));
}

void ControlBoardsController::start_main_task()
{
    TaskHandle_t control_boards_task_handle;
    xTaskCreate(_main_task_cb, "ControlBoardsController main", 3500, this, 1, &control_boards_task_handle);
}

void ControlBoardsController::start_display_only_task()
{
    TaskHandle_t control_boards_task_handle;
    xTaskCreate(_display_only_task_cb, "ControlBoardsController display_only", 3500, this, 1, &control_boards_task_handle);
}

void ControlBoardsController::main_task()
{
    while (true)
    {
        _talk_to_controls_main();
        _talk_to_controls_pages();
        vTaskDelay(pdMS_TO_TICKS(I2C_CONFIG_TRANSACTION_DELAY_US / 1000));
    }
}

void ControlBoardsController::display_only_task()
{
    esp_err_t err;

    while (true)
    {
        do {
            err = _transmit_controls_main_display(_get_controls_main_display());
        } while (err != ESP_OK);

        do {
            err = _transmit_controls_pages_display(_get_controls_pages_display());
        } while (err != ESP_OK);

        vTaskDelay(pdMS_TO_TICKS(I2C_CONFIG_TRANSACTION_DELAY_US / 1000));
    }
}

void ControlBoardsController::_talk_to_controls_main()
{
    esp_err_t err;

    do {
        err = _read_controls_main_events();
    } while (err != ESP_OK);

    do {
        err = _transmit_controls_main_display(_get_controls_main_display());
    } while (err != ESP_OK);
}

esp_err_t ControlBoardsController::_read_controls_main_events()
{
    esp_err_t err = ESP_OK;

    controls_main_value_t controls_main_value;
    err = i2c_master_receive(_controls_main_device_handle, (uint8_t *) &controls_main_value, sizeof(controls_main_value_t), -1);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "controls_main receive err: %x", err);
        return err;
    }

    if (!check_crc16((uint8_t *) &controls_main_value, sizeof(controls_main_value_t)))
    {
        err = ESP_ERR_INVALID_CRC;
        ESP_LOGE(TAG, "controls_main CRC check FAILED!");
        return err;
    }

    _listener.handle_controls_main_event(controls_main_value);
    _last_controls_main_value = controls_main_value;

    return err;
}

static uint8_t _push_buttons_events_to_consumed_clicks(uint16_t push_buttons_events, const uint8_t num_buttons)
{
    uint8_t consumed_clicks = 0;

    for (size_t i = num_buttons; i > 0; i--)
    {
        // odd number of clicks consumed
        if (push_buttons_events & 0x2)
        {
            consumed_clicks = consumed_clicks | 0x80;
        }

        if (i <= 1)
        {
            break;
        }

        push_buttons_events = push_buttons_events >> 2;
        consumed_clicks = consumed_clicks >> 1;
    }
    
    return consumed_clicks;
}

esp_err_t ControlBoardsController::_transmit_controls_main_display(controls_main_display_t controls_main_display)
{
    esp_err_t err = ESP_OK;

    memcpy(_main_write_buffer, &controls_main_display, sizeof(controls_main_display_t));
    compute_crc16(_main_write_buffer, sizeof(controls_main_display_t));

    err = i2c_master_transmit(_controls_main_device_handle, _main_write_buffer, sizeof(controls_main_display_t), -1);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "controls_main transmit err: %x", err);
        return err;
    }

    return err;
}

controls_main_display_t ControlBoardsController::_get_controls_main_display()
{
    controls_main_display_t controls_main_display = _listener.get_controls_main_display();

    controls_main_display.tracks_buttons_clicks_consumed = _push_buttons_events_to_consumed_clicks(_last_controls_main_value.tracks_push_buttons, SEQUENCER_TRACKS_NUM);

    return controls_main_display;
}

void ControlBoardsController::_talk_to_controls_pages()
{
    esp_err_t err;

    do {
        err = _read_controls_pages_events();
    } while (err != ESP_OK);

    do {
        err = _transmit_controls_pages_display(_get_controls_pages_display());
    } while (err != ESP_OK);
}

esp_err_t ControlBoardsController::_read_controls_pages_events()
{
    esp_err_t err = ESP_OK;

    controls_pages_value_t controls_pages_value;
    err = i2c_master_receive(_controls_pages_device_handle, (uint8_t *) &controls_pages_value, sizeof(controls_pages_value_t), -1);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "controls_pages receive err: %x", err);
        return err;
    }

    if (!check_crc16((uint8_t *) &controls_pages_value, sizeof(controls_pages_value_t)))
    {
        err = ESP_ERR_INVALID_CRC;
        ESP_LOGE(TAG, "controls_pages CRC check FAILED!");
        return err;
    }

    _listener.handle_controls_pages_event(controls_pages_value);
    _last_controls_pages_value = controls_pages_value;

    return err;
}

esp_err_t ControlBoardsController::_transmit_controls_pages_display(controls_pages_display_t controls_pages_display)
{
    esp_err_t err = ESP_OK;

    memcpy(_pages_write_buffer, &controls_pages_display, sizeof(controls_pages_display_t));
    compute_crc16(_pages_write_buffer, sizeof(controls_pages_display_t));

    err = i2c_master_transmit(_controls_pages_device_handle, _pages_write_buffer, sizeof(controls_pages_display_t), -1);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "controls_pages transmit err: %x", err);
        return err;
    }

    return err;
}

controls_pages_display_t ControlBoardsController::_get_controls_pages_display()
{
    controls_pages_display_t controls_pages_display = _listener.get_controls_pages_display();

    controls_pages_display.played_pages_buttons_clicks_consumed = _push_buttons_events_to_consumed_clicks(_last_controls_pages_value.played_pages_buttons, SEQUENCER_TRACKS_NUM);

    controls_pages_display.edited_pages_buttons_clicks_consumed = _push_buttons_events_to_consumed_clicks(_last_controls_pages_value.edited_pages_buttons, SEQUENCER_TRACKS_NUM);

    return controls_pages_display;
}
