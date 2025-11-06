#include "PlayPauseBPMLED.h"

void IRAM_ATTR _bpm_timer_callback()
{
    xSemaphoreGiveFromISR(PlayPauseBPMLED::instance->bpm_timer_semaphore, NULL);
}

static void _bpm_task_cb(void *arg)
{
    PlayPauseBPMLED *play_pause_bpm_led = (PlayPauseBPMLED *)arg;
    play_pause_bpm_led->bpm_task();
}

PlayPauseBPMLED *PlayPauseBPMLED::instance;

PlayPauseBPMLED::PlayPauseBPMLED()
{
    PlayPauseBPMLED::instance = this;

    pinMode(PLAY_PAUSE_LED_GPIO, OUTPUT);

    _is_enabled = false;
    _bpm = -1.;
    _bpm_led_state = false;
    _bpm_timer = NULL;
    _bpm_timer_last_alarm = 0;
    bpm_timer_semaphore = xSemaphoreCreateBinary();

    disable();

    TaskHandle_t _bpm_task_handle;
    xTaskCreate(_bpm_task_cb, "BPM LED task", 2048, this, 3, &_bpm_task_handle);
}

void PlayPauseBPMLED::bpm_task()
{
    while (true)
    {
        if (xSemaphoreTake(bpm_timer_semaphore, portMAX_DELAY) == pdTRUE){
            _handle_bpm_timer_tick();
        }
    }
}

bool PlayPauseBPMLED::is_enabled()
{
    return _is_enabled;
}

void PlayPauseBPMLED::set_enabled(bool enabled)
{
    if (enabled != _is_enabled)
    {
        if (enabled)
        {
            enable();
        }
        else
        {
            disable();
        }
    }
}

void PlayPauseBPMLED::update(bool led_enabled, float bpm)
{
    if (bpm > 0)
    {
        if (_bpm < 0)
        {
            _start_bpm_blink();
        }
        else if (!_bpm_led_state)
        {
            _bpm = bpm;
            _reschedule_next_blink();
        }
    }
    else
    {
        if (_bpm >= 0)
        {
            _stop_bpm_blink();
        }
    }
    _bpm = bpm;

    if (_bpm < 0)
    {
        enable();
    }
}

void PlayPauseBPMLED::enable()
{
    digitalWrite(PLAY_PAUSE_LED_GPIO, HIGH);
    _is_enabled = true;
}

void PlayPauseBPMLED::disable()
{
    digitalWrite(PLAY_PAUSE_LED_GPIO, LOW);
    _is_enabled = false;
}

void PlayPauseBPMLED::_handle_bpm_timer_tick()
{
    _bpm_timer_last_alarm = 0;
    if (_bpm > 0)
    {
        _bpm_led_state = !_bpm_led_state;
        digitalWrite(PLAY_PAUSE_LED_GPIO, _bpm_led_state ? HIGH : LOW);

        uint64_t next_alarm = _get_next_bpm_timer_alarm();

        // Update timer alarm
        timerRestart(_bpm_timer);
        timerAlarmWrite(_bpm_timer, next_alarm, false);
        timerAlarmEnable(_bpm_timer);
    }
    else
    {
        // If BPM is not set, turn off LED and stop timer
        _stop_bpm_blink();
    }
}

uint64_t PlayPauseBPMLED::_get_next_bpm_timer_alarm()
{
    uint64_t next_alarm;
    float interval_seconds = 60.0f / (_bpm / 2.); // Divide by 2 because _bpm is about 8th notes. We want to display quarter notes BPM.
    uint64_t interval_micros = static_cast<uint64_t>(interval_seconds * 1e6);

    if (interval_micros < PLAY_PAUSE_LED_BLINK_US * 2)
    {
        next_alarm = interval_micros / 2;
    }
    else
    {
        if (!_bpm_led_state)
        {
            next_alarm = interval_micros - PLAY_PAUSE_LED_BLINK_US;
        }
        else
        {
            next_alarm = PLAY_PAUSE_LED_BLINK_US;
        }
    }

    return next_alarm;
}

void PlayPauseBPMLED::_start_bpm_blink()
{
    if (!_bpm_timer)
    {
        _bpm_timer = timerBegin(2, 80, true);
        timerAttachInterrupt(_bpm_timer, &_bpm_timer_callback, false);
    }
    
    timerRestart(_bpm_timer);
    timerAlarmWrite(_bpm_timer, 50, false);
    timerAlarmEnable(_bpm_timer);
}

void PlayPauseBPMLED::_reschedule_next_blink()
{
    uint64_t next_alarm = _get_next_bpm_timer_alarm();
    uint64_t timer_count = timerReadMicros(_bpm_timer);
    _bpm_timer_last_alarm += timer_count;
    int64_t alarm_value = next_alarm - _bpm_timer_last_alarm;
    if (alarm_value < 50)
    {
        alarm_value = 50;
    }

    timerRestart(_bpm_timer);
    timerAlarmWrite(_bpm_timer, alarm_value, false);
    timerAlarmEnable(_bpm_timer);
}

void PlayPauseBPMLED::_stop_bpm_blink()
{
    _bpm_led_state = false;
    digitalWrite(PLAY_PAUSE_LED_GPIO, LOW);
    timerAlarmDisable(_bpm_timer);
}