#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <esp32-hal-timer.h>

#define PLAY_PAUSE_LED_GPIO 23
#define PLAY_PAUSE_LED_BLINK_US 50000

class PlayPauseBPMLED
{
public:
    PlayPauseBPMLED();

    PlayPauseBPMLED static *instance;
    
    void bpm_task();
    bool is_enabled();
    void set_enabled(bool enabled);
    void update(bool led_enabled, float bpm);
    void enable();
    void disable();
    
    xSemaphoreHandle bpm_timer_semaphore;
    
    private:
    bool _is_enabled;
    float _bpm;
    
    bool _bpm_led_state;
    hw_timer_t* _bpm_timer;
    uint64_t _bpm_timer_last_alarm;
    
    void _handle_bpm_timer_tick();
    uint64_t _get_next_bpm_timer_alarm();
    void _start_bpm_blink();
    void _reschedule_next_blink();
    void _stop_bpm_blink();
};