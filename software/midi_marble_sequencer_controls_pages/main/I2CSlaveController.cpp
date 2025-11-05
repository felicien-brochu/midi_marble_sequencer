#include "I2CSlaveController.h"

#include <crc.h>
#include <cstring>
#include <esp_log.h>

static const char *TAG = "I2CSlaveController";

static i2c_slave_rx_done_event_data_t i2c_event_rx_data;

static IRAM_ATTR bool i2c_slave_on_receive_callback(i2c_slave_dev_handle_t i2c_slave, const i2c_slave_rx_done_event_data_t *event_data, void *queue)
{
    i2c_event_type_t event_type = I2C_EVENT_TYPE_RX;
    i2c_event_rx_data = *event_data;
    BaseType_t high_task_wakeup = pdFALSE;
    QueueHandle_t event_queue = (QueueHandle_t)queue;
    xQueueSendFromISR(event_queue, &event_type, &high_task_wakeup);
    return high_task_wakeup == pdTRUE;
}

static IRAM_ATTR bool i2c_slave_on_request_callback(i2c_slave_dev_handle_t i2c_slave, const i2c_slave_request_event_data_t *evt_data, void *queue)
{
    i2c_event_type_t event_type = I2C_EVENT_TYPE_TX;
    BaseType_t high_task_wakeup = pdFALSE;
    QueueHandle_t event_queue = (QueueHandle_t)queue;
    xQueueSendFromISR(event_queue, &event_type, &high_task_wakeup);
    return high_task_wakeup == pdTRUE;
}


I2CSlaveController::I2CSlaveController(InteractionController *interaction_controller, DisplayController *display_controller) : _interaction_controller(interaction_controller), _display_controller(display_controller)
{    
    i2c_slave_config_t i2c_slave_config = {
        .i2c_port = I2C_CONFIG_PORT,
        .sda_io_num = I2C_SLAVE_SDA_GPIO,
        .scl_io_num = I2C_SLAVE_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .send_buf_depth = I2C_SEND_BUF_DEPTH,
        .receive_buf_depth = I2C_RECEIVE_BUF_DEPTH,
        .slave_addr = I2C_CONFIG_CONTROLS_PAGES_ADDR,
        .addr_bit_len = I2C_ADDR_BIT_LEN_7,
        .flags = {
            .allow_pd = true,
            .enable_internal_pullup = false,
        },
    };

    ESP_ERROR_CHECK(i2c_new_slave_device(&i2c_slave_config, &_slave_handle));

    _event_queue = xQueueCreate(1, sizeof(i2c_event_type_t));
    i2c_slave_event_callbacks_t callbacks = {
        .on_request = i2c_slave_on_request_callback,
        .on_receive = i2c_slave_on_receive_callback,
    };
    ESP_ERROR_CHECK(i2c_slave_register_event_callbacks(_slave_handle, &callbacks, _event_queue));
}

void I2CSlaveController::main_task()
{
    while (true)
    {
        i2c_event_type_t i2c_event_type = I2C_EVENT_TYPE_NONE;
        bool retrieved_event_from_queue = xQueueReceive(_event_queue, &i2c_event_type, pdMS_TO_TICKS(10000));

        if (retrieved_event_from_queue) {
            if (i2c_event_type == I2C_EVENT_TYPE_RX)
            {
                _on_receive(&i2c_event_rx_data);
            }
            else if (i2c_event_type == I2C_EVENT_TYPE_TX)
            {
                _on_request();
            }
        }
        else
        {
            ESP_LOGE(TAG, "i2c event queue failed");
        }
    }
}

void I2CSlaveController::_on_receive(const i2c_slave_rx_done_event_data_t *event_data)
{
    uint8_t *rx_buffer = event_data->buffer;
    bool crc_check_ok = check_crc16(rx_buffer, sizeof(controls_pages_display_t));

    if (!crc_check_ok)
    {
        ESP_LOGE(TAG, "error on receive: CRC check FAILED");
        return;
    }

    controls_pages_display_t controls_pages_display;
    memcpy(&controls_pages_display, rx_buffer, sizeof(controls_pages_display_t));

    _display_controller->update(controls_pages_display);
}

void I2CSlaveController::_on_request()
{
    ESP_LOGI(TAG, "on request");
    controls_pages_value_t controls_pages_value;
    _interaction_controller->get_value(&controls_pages_value);
    uint8_t before_crc = controls_pages_value.played_pages_buttons[7].click_events_pending;
    compute_crc16((uint8_t *) &controls_pages_value, sizeof(controls_pages_value_t));
    uint32_t write_len;
    esp_err_t err;
    err = i2c_slave_write(_slave_handle, (uint8_t *) &controls_pages_value, sizeof(controls_pages_value_t), &write_len, -1);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "error on write: %d", err);
        return;
    }
    

    // controls_pages_value_t *value_from_buffer = (controls_pages_value_t *)_write_buffer;

    // ESP_LOGI(TAG, "respond value: played_pages[7]: %d copied %d", _interaction_controller->_played_pages_buttons_controller._push_buttons_events[7].click_events_pending, controls_pages_value.played_pages_buttons[7].click_events_pending);

    // if (before_crc != controls_pages_value.played_pages_buttons[7].click_events_pending) {
    //     ESP_LOGE(TAG, "Diff before %d, after %d", before_crc, controls_pages_value.played_pages_buttons[7].click_events_pending);
    // }


    for (size_t i = 0; i < 8; i++)
    {
        if (controls_pages_value.played_pages_buttons[i].click_events_pending > 0) {
            ESP_LOGI(TAG, "%d clicked", i);
        }
    }

    _interaction_controller->consume_events();
}