#include "I2CSlaveController.h"
#include <cstring>

static IRAM_ATTR bool i2c_slave_rx_done_callback(i2c_slave_dev_handle_t channel, const i2c_slave_rx_done_event_data_t *event_data, void *queue)
{
    BaseType_t high_task_wakeup = pdFALSE;
    QueueHandle_t receive_queue = (QueueHandle_t)queue;
    xQueueSendFromISR(receive_queue, event_data, &high_task_wakeup);
    return high_task_wakeup == pdTRUE;
}


I2CSlaveController::I2CSlaveController(InteractionController &interaction_controller, DisplayController &display_controller) : _interaction_controller(interaction_controller), _display_controller(display_controller)
{
    _write_buffer = (uint8_t *) malloc(sizeof(controls_main_value_t));
    _read_buffer = (uint8_t *) malloc(sizeof(controls_main_display_t));
    
    i2c_slave_config_t i2c_slave_config = {
        .i2c_port = I2C_CONFIG_PORT,
        .sda_io_num = I2C_SLAVE_SDA_GPIO,
        .scl_io_num = I2C_SLAVE_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .send_buf_depth = I2C_SEND_BUF_DEPTH,
        .slave_addr = I2C_CONFIG_CONTROLS_MAIN_ADDR,
        .addr_bit_len = I2C_ADDR_BIT_LEN_7,
    };

    ESP_ERROR_CHECK(i2c_new_slave_device(&i2c_slave_config, &_slave_handle));

    _receive_queue = xQueueCreate(1, sizeof(i2c_slave_rx_done_event_data_t));
    i2c_slave_event_callbacks_t callbacks = {
        .on_recv_done = i2c_slave_rx_done_callback,
    };
    ESP_ERROR_CHECK(i2c_slave_register_event_callbacks(_slave_handle, &callbacks, _receive_queue));
}

void I2CSlaveController::main_task()
{
    while (true)
    {
        i2c_slave_rx_done_event_data_t rx_done_event_data;
        ESP_ERROR_CHECK(i2c_slave_receive(_slave_handle, _read_buffer, sizeof(_controls_main_display)));
        bool rx_success = xQueueReceive(_receive_queue, &rx_done_event_data, pdMS_TO_TICKS(10000));

        if (rx_success) {
            memcpy(&_controls_main_display, rx_done_event_data.buffer, sizeof(_controls_main_display));
            
            // printf("RECEIVED _controls_main_display: play_pause_button: %d, tracks_led_enabled: %d, %d, %d, %d\n", _controls_main_display.play_pause_led_enabled, _controls_main_display.tracks_led_enabled[0], _controls_main_display.tracks_led_enabled[1], _controls_main_display.tracks_led_enabled[2], _controls_main_display.tracks_led_enabled[3]);
            

            _display_controller.update(_controls_main_display);
            _controls_main_value = _interaction_controller.get_value();

            // printf("SEND _controls_main_value: play_pause_switch_pushed: %d, push_buttons: %d, %d, %d, %d, %d, %d, %d, %d\n", _controls_main_value.play_pause_switch_pushed, 
            //     _controls_main_value.tracks_push_buttons[0].click_events_pending,
            //     _controls_main_value.tracks_push_buttons[1].click_events_pending,
            //     _controls_main_value.tracks_push_buttons[2].click_events_pending,
            //     _controls_main_value.tracks_push_buttons[3].click_events_pending,
            //     _controls_main_value.tracks_push_buttons[4].click_events_pending,
            //     _controls_main_value.tracks_push_buttons[5].click_events_pending,
            //     _controls_main_value.tracks_push_buttons[6].click_events_pending,
            //     _controls_main_value.tracks_push_buttons[7].click_events_pending
            // );

            memcpy(_write_buffer, &_controls_main_value, sizeof(_controls_main_value));
            ESP_ERROR_CHECK(i2c_slave_transmit(_slave_handle, _write_buffer, sizeof(_controls_main_value), -1));

            _interaction_controller.consume_events();

            // printf(".");
        }
        else
        {
            printf("QUEUE FAIL\n");
        }
        // vTaskDelay(pdMS_TO_TICKS(10));
    }
}


