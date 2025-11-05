#include "I2CSlaveController.h"

#include <Arduino.h>
#include <crc.h>
#include <Wire.h>

 
void IRAM_ATTR timer0_isr()
{
    xSemaphoreGiveFromISR(I2CSlaveController::instance->timer_semaphore, NULL);
}

static void on_receive_cb(int len)
{
    I2CSlaveController::instance->on_receive(len);
}

static void on_request_cb()
{
    I2CSlaveController::instance->on_request();
}

I2CSlaveController *I2CSlaveController::instance;

I2CSlaveController::I2CSlaveController(InteractionController *interaction_controller, DisplayController *display_controller) : _interaction_controller(interaction_controller), _display_controller(display_controller)
{
    I2CSlaveController::instance = this;

    _read_buffer = (uint8_t *) malloc(sizeof(controls_pages_display_t));

    _requests_since_last_read = 0;
    _write_timer = NULL;
    _write_buffer = (uint8_t *) malloc(sizeof(controls_pages_value_t));
    _write_buffer_lock = xSemaphoreCreateMutex();
    timer_semaphore = xSemaphoreCreateBinary();
    Wire.onReceive(on_receive_cb);
	Wire.onRequest(on_request_cb);
    while(!Wire.begin((uint8_t) I2C_CONFIG_CONTROLS_PAGES_ADDR)) {}
}

void I2CSlaveController::main_task()
{
    while (true)
    {
        if (xSemaphoreTake(timer_semaphore, portMAX_DELAY) == pdTRUE){
            write_to_buffer();
        }
    }
}

void I2CSlaveController::on_receive(int len)
{
    for (size_t i = 0; i < sizeof(controls_pages_display_t) && Wire.available(); i++)
    {
		_read_buffer[i] = (uint8_t) Wire.read();
    }
    
    while (Wire.available()) {
		Wire.read();
	}
    
    _schedule_next_write();
    _requests_since_last_read = 0;



    bool crc_check_ok = check_crc16(_read_buffer, sizeof(controls_pages_display_t));

    if (!crc_check_ok)
    {
        log_e("error on receive: CRC check FAILED");
        return;
    }

    controls_pages_display_t controls_pages_display;
    memcpy(&controls_pages_display, _read_buffer, sizeof(controls_pages_display_t));

    _display_controller->update(controls_pages_display);
    _interaction_controller->consume_events(controls_pages_display);
}

void I2CSlaveController::on_request()
{
    if (_requests_since_last_read > 0) {
        write_to_buffer();
    }
    _requests_since_last_read++;
}

void I2CSlaveController::_schedule_next_write()
{
    if (!_write_timer)
    {
        _write_timer = timerBegin(0, 80, true);
        timerAttachInterrupt(_write_timer, &timer0_isr, false);
        timerAlarmWrite(_write_timer, I2C_CONFIG_MIN_TRANSACTION_TIME_US - I2C_CONFIG_SLAVE_FIFO_DELAY_US, false);
        timerAlarmEnable(_write_timer);
    }
    else
    {
        timerRestart(_write_timer);
        timerAlarmEnable(_write_timer);
    }
}

void I2CSlaveController::write_to_buffer()
{
    xSemaphoreTake(_write_buffer_lock, portMAX_DELAY);
    Wire.slaveWrite(_write_buffer, sizeof(controls_pages_value_t));
    xSemaphoreGive(_write_buffer_lock);
}

void I2CSlaveController::on_event(controls_pages_value_t controls_pages_value)
{
    xSemaphoreTake(_write_buffer_lock, portMAX_DELAY);
    memcpy(_write_buffer, &controls_pages_value, sizeof(controls_pages_value_t));
	compute_crc16(_write_buffer, sizeof(controls_pages_value_t));
    xSemaphoreGive(_write_buffer_lock);
}