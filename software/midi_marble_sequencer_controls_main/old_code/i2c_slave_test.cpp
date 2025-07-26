#include "i2c_slave_test.h"
#include <stdlib.h>
#include "driver/i2c_slave.h"
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>

#define WRITE_LEN 10
#define READ_LEN 1

static IRAM_ATTR bool i2c_slave_rx_done_callback(i2c_slave_dev_handle_t channel, const i2c_slave_rx_done_event_data_t *edata, void *user_data)
{
    BaseType_t high_task_wakeup = pdFALSE;
    QueueHandle_t receive_queue = (QueueHandle_t)user_data;
    xQueueSendFromISR(receive_queue, edata, &high_task_wakeup);
    return high_task_wakeup == pdTRUE;
}

void main_test_i2c_slave() {
    gpio_reset_pin(GPIO_NUM_21);
    gpio_reset_pin(GPIO_NUM_22);
    
    i2c_slave_config_t i2c_slv_config = {
        .i2c_port = 0,
        .sda_io_num = GPIO_NUM_21,
        .scl_io_num = GPIO_NUM_22,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .send_buf_depth = 256,
        .slave_addr = 0x57,
        .addr_bit_len = I2C_ADDR_BIT_LEN_7,
        .intr_priority = 3,
        };

    i2c_slave_dev_handle_t slave_handle;
    ESP_ERROR_CHECK(i2c_new_slave_device(&i2c_slv_config, &slave_handle));

    uint8_t *data_wr = (uint8_t *)malloc(WRITE_LEN * sizeof(uint8_t));
    uint8_t *data_rd = (uint8_t *)malloc(READ_LEN * sizeof(uint8_t));
    data_rd[0] = 2;
    uint32_t size_rd = 0;

    QueueHandle_t s_receive_queue = xQueueCreate(1, sizeof(i2c_slave_rx_done_event_data_t));
    i2c_slave_event_callbacks_t cbs = {
        .on_recv_done = i2c_slave_rx_done_callback,
    };
    ESP_ERROR_CHECK(i2c_slave_register_event_callbacks(slave_handle, &cbs, s_receive_queue));

    while (1)
    {
        printf("####receive and transmit\n");
        for (int i = 0; i < WRITE_LEN; i++)
        {
            data_wr[i] = i;
        }

        i2c_slave_rx_done_event_data_t rx_data;
        ESP_ERROR_CHECK(i2c_slave_receive(slave_handle, data_rd, READ_LEN));
        xQueueReceive(s_receive_queue, &rx_data, pdMS_TO_TICKS(10000));

        printf("###rx_data[0]: %d\n", rx_data.buffer[0]);

        ESP_ERROR_CHECK(i2c_slave_transmit(slave_handle, data_wr, WRITE_LEN, 10000));
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Receive done.