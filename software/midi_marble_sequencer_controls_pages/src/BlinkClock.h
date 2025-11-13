#pragma once
#include <cstdint>
#include <esp32-hal-timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#define BLINK_CLOCK_MAX_LISTENERS 8

class IBlinkClockListener
{
public:

    virtual void on_blink_clock_tick(bool is_on) = 0;
};

class BlinkClock
{
public:
    // Access the singleton instance
    static BlinkClock& instance() {
        static BlinkClock inst(500000, 500000, 2);
        return inst;
    }

    // non-copyable, non-movable
    BlinkClock(const BlinkClock&) = delete;
    BlinkClock& operator=(const BlinkClock&) = delete;
    BlinkClock(BlinkClock&&) = delete;
    BlinkClock& operator=(BlinkClock&&) = delete;


    void add_listener(IBlinkClockListener *listener);
    void main_task();
    
    xSemaphoreHandle timer_semaphore;
    
    private:
    BlinkClock(uint32_t on_time, uint32_t off_time, uint8_t timer_id);
    
    IBlinkClockListener *_listeners[BLINK_CLOCK_MAX_LISTENERS];
    uint8_t _listeners_num;

    uint32_t _on_time;
    uint32_t _off_time;
    uint8_t _timer_id;
    
    bool _is_on;
    hw_timer_t* _blink_timer;
    uint64_t _bpm_timer_last_alarm;
    
    void _handle_tick();
    void _start();
};