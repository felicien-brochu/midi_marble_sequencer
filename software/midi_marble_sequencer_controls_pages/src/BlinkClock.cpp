#include "BlinkClock.h"

void IRAM_ATTR _blink_timer_callback()
{
    xSemaphoreGiveFromISR(BlinkClock::instance().timer_semaphore, NULL);
}


static void _blink_task_cb(void *arg)
{
    BlinkClock *blink_clock = (BlinkClock *)arg;
    blink_clock->main_task();
}

BlinkClock::BlinkClock(uint32_t on_time, uint32_t off_time, uint8_t timer_id)
{
    _on_time = on_time;
    _off_time = off_time;
    _timer_id = timer_id;

    _is_on = false;

    _listeners_num = 0;

    timer_semaphore = xSemaphoreCreateBinary();
    TaskHandle_t _blink_task_handle;
    xTaskCreate(_blink_task_cb, "BlinkClock", 2048, this, 3, &_blink_task_handle);

    _blink_timer = NULL;

    _start();
}

void BlinkClock::add_listener(IBlinkClockListener *listener)
{
    if (_listeners_num < BLINK_CLOCK_MAX_LISTENERS - 1)
    {
        _listeners[_listeners_num] = listener;
        _listeners_num++;
    }
}

void BlinkClock::_start()
{
    if (!_blink_timer)
    {
        _blink_timer = timerBegin(_timer_id, 80, true);
        timerAttachInterrupt(_blink_timer, &_blink_timer_callback, false);
    }
    
    timerRestart(_blink_timer);
    timerAlarmWrite(_blink_timer, 50, false);
    timerAlarmEnable(_blink_timer);
}

void BlinkClock::main_task()
{
    while (true)
    {
        if (xSemaphoreTake(timer_semaphore, portMAX_DELAY) == pdTRUE) {
            _handle_tick();
        }
    }
}

void BlinkClock::_handle_tick()
{
    _is_on = !_is_on;

    uint32_t next_tick = _is_on ? _on_time : _off_time;
    timerRestart(_blink_timer);
    timerAlarmWrite(_blink_timer, next_tick, false);
    timerAlarmEnable(_blink_timer);

    for (size_t i = 0; i < _listeners_num; i++)
    {
        _listeners[i]->on_blink_clock_tick(_is_on);
    }
}
