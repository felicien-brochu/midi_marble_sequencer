#include "i2c_master_test.h"
#include <stdlib.h>
#include "driver/i2c_master.h"
#include <freertos/FreeRTOS.h>

#define DATA_LENGTH 100

void main_test_i2c_master()
{
    i2c_master_bus_config_t i2c_mst_config = {
        .i2c_port = 0,
        .sda_io_num = GPIO_NUM_5,
        .scl_io_num = GPIO_NUM_4,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
    };
    i2c_master_bus_handle_t bus_handle;

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x57,
        .scl_speed_hz = 100000,
    };

    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));

    uint8_t write_buffer[1] = {127};
    uint8_t read_buffer[10];

    while (1)
    {
        ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle, write_buffer, sizeof(write_buffer), read_buffer, 10, -1));
        for (size_t i = 0; i < 10; i++)
        {
            printf("Data%d: %d\n", i, read_buffer[i]);
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}