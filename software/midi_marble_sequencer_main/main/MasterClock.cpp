#include "MasterClock.h"

#include <esp_err.h>
#include <esp_log.h>
#include <freertos/freeRTOS.h>

static const char *TAG = "MasterClock";

static void sequencer_callback(sequencer_callback_type_t callback_type, void *arg, void *context)
{
    MasterClock *master_clock = (MasterClock *)context;

    switch (callback_type)
    {
    case SEQUENCER_CB_START_PLAYING:
        master_clock->handle_start_playing_event();
        break;
    case SEQUENCER_CB_STOP_PLAYING:
        master_clock->handle_stop_playing_event();
        break;
    case SEQUENCER_CB_NEW_LOCK_EVENT:
        master_clock->handle_new_lock_event();
        break;
    default:
        break;
    }
}

static void timed_queue_task(void *master_clock_arg)
{
    MasterClock *master_clock = (MasterClock *) master_clock_arg;
    master_clock->execute_timed_queue_requests();
}

static void read_newly_locked_measures_task(void *master_clock_arg)
{
    MasterClock *master_clock = (MasterClock *) master_clock_arg;
    master_clock->read_newly_locked_measures();

    vTaskDelete(NULL);
}

static bool IRAM_ATTR _timing_clock_callback(gptimer_handle_t gptimer, const gptimer_alarm_event_data_t *event_data, void *user_data)
{
    MasterClock *master_clock = (MasterClock *) user_data;
    return master_clock->handle_timing_clock_event();
}

MasterClock::MasterClock(Sequencer &sequencer) : _sequencer(sequencer), _control_boards_controller(sequencer), _midi_controller(sequencer)
{
    _timed_queue = xQueueCreate(10, sizeof(timed_queue_request_t));
    _alarms_since_eighth_note = 0;
    _eighth_note_marble_detected = false;
    _measure_lock_event_queue = xQueueCreate(4, sizeof(bool));

    _start_timing_clock();
    _sequencer.set_sequencer_callback(sequencer_callback, this);
    _control_boards_controller.start_main_task();
}

void MasterClock::_start_timing_clock()
{
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT, // Select the default clock source
        .direction = GPTIMER_COUNT_UP,      // Counting direction is up
        .resolution_hz = 1 * 1000 * 1000,   // Resolution is 1 MHz, i.e., 1 tick equals 1 microsecond
    };
    
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &_gptimer));

    gptimer_alarm_config_t alarm_config = {
        .alarm_count = 1, // Set the actual alarm period, since the resolution is 1us, 1000000 represents 1s
        .reload_count = 0,
        .flags = {
            .auto_reload_on_alarm = true
        }
    };
    // Set the timer's alarm action
    ESP_ERROR_CHECK(gptimer_set_alarm_action(_gptimer, &alarm_config));

    gptimer_event_callbacks_t callbacks = {
        .on_alarm = _timing_clock_callback, // Call the user callback function when the alarm event occurs
    };
    // Register timer event callback functions, allowing user context to be carried
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(_gptimer, &callbacks, this));
    ESP_ERROR_CHECK(gptimer_enable(_gptimer));
    ESP_ERROR_CHECK(gptimer_start(_gptimer));

    xTaskCreate(timed_queue_task, "timed_queue", 3600, this, 1, &_timed_queue_task_handle);
    xTaskCreate(read_newly_locked_measures_task, "read_locked_measures", 3600, this, 3, &_read_locked_measures_task_handle);
}

BaseType_t MasterClock::handle_timing_clock_event()
{
    BaseType_t high_task_awoken = pdFALSE;
    
    _alarms_since_eighth_note++;
    uint64_t tick_duration = _get_tick_duration();

    timed_queue_request_t timing_clock_req = SEND_TIMING_CLOCK_REQ;
    xQueueSendFromISR(_timed_queue, &timing_clock_req, &high_task_awoken);

    if (_sequencer.is_playing())
    {
        timed_queue_request_t timed_queue_request = NO_REQ;
        
        if (_alarms_since_eighth_note == TIMING_CLOCK_EVENT_BY_EIGHTH_NOTE)
        {
            timed_queue_request = SEND_MIDI_NOTES_REQ;
        }
        else if (!_eighth_note_marble_detected)
        {
            uint64_t time_until_send_notes = (TIMING_CLOCK_EVENT_BY_EIGHTH_NOTE - _alarms_since_eighth_note) * tick_duration;
            if (time_until_send_notes - tick_duration <= DETECT_EIGHTH_NOTE_MARBLES_DURATION)
            {
                timed_queue_request = DETECT_MARBLES_REQ;
                _eighth_note_marble_detected = true;
            }
        }

        if (timed_queue_request != NO_REQ)
        {
            xQueueSendFromISR(_timed_queue, &timed_queue_request, &high_task_awoken);
        }
    }

    if (_alarms_since_eighth_note >= TIMING_CLOCK_EVENT_BY_EIGHTH_NOTE)
    {
        _alarms_since_eighth_note = 0;
        _eighth_note_marble_detected = false;
    }

    gptimer_alarm_config_t alarm_config = {
        .alarm_count = tick_duration, // Set the actual alarm period, since the resolution is 1us, 1000000 represents 1s
        .reload_count = 0,
        .flags = {
            .auto_reload_on_alarm = true
        }
    };
    // Set the timer's alarm action
    ESP_ERROR_CHECK(gptimer_set_alarm_action(_gptimer, &alarm_config));

    // return whether we need to yield at the end of ISR
    return (high_task_awoken == pdTRUE);
}

void MasterClock::execute_timed_queue_requests()
{
    timed_queue_request_t request;
    while (true) {
        if (xQueueReceive(_timed_queue, &request, pdMS_TO_TICKS(4000))) {
            if (request == DETECT_MARBLES_REQ)
            {
                _execute_detect_marbles_request();
            }
            else if (request == SEND_MIDI_NOTES_REQ)
            {
                _execute_send_midi_notes_request();
            }
            else if (request == SEND_TIMING_CLOCK_REQ)
            {
                _execute_send_timing_clock_request();
            }
        }
    }
}

void MasterClock::_execute_detect_marbles_request()
{
    _detect_marbles();
    _led_snake.set_eighth_note_index(_sequencer.get_current_eighth_note_index());
}

void MasterClock::_execute_send_midi_notes_request()
{
    _midi_controller.send_eighth_note_midi_notes();
    _led_snake.update();
    _sequencer.next_eighth_note();
}

void MasterClock::_execute_send_timing_clock_request()
{
    _midi_controller.send_timing_clock();
}

uint64_t MasterClock::_get_tick_duration()
{
    return _sequencer.get_eighth_note_duration() / TIMING_CLOCK_EVENT_BY_EIGHTH_NOTE;
}

void MasterClock::start_playing()
{

    _led_snake.set_eighth_note_index(_sequencer.get_current_eighth_note_index());
    _led_snake.set_enabled(true);
}

void MasterClock::stop_playing()
{
    _midi_controller.send_notes_off();

    _led_snake.set_enabled(false);
    _led_snake.update();
}

void MasterClock::_detect_marbles()
{
    if (!_sequencer.is_current_eighth_note_locked())
    {
        marble_type_t marble_types[SEQUENCER_TRACKS_NUM];
        _marble_detector.detect_eighth_note_marbles(_sequencer.get_current_eighth_note_index(), marble_types);

        _sequencer.set_current_eighth_note_marble_types(marble_types);

        // for (size_t i = 0; i < SEQUENCER_TRACKS_NUM; i++)
        // {
        //     if (marble_types[i] != NO_MARBLE)
        //     {
        //         ESP_LOGI(TAG, "%d[%d]: %s", _sequencer.get_current_eighth_note_index(), i, marble_type_to_string(marble_types[i]));
        //     }
        // }
    }
}

void MasterClock::schedule_read_newly_locked_measures()
{
    bool sent = true;
    xQueueSend(_measure_lock_event_queue, &sent, 0);
}

void MasterClock::read_newly_locked_measures()
{
    bool received;

    while (true) {
        if (xQueueReceive(_measure_lock_event_queue, &received, pdMS_TO_TICKS(4000))) {
            for (size_t i = 0; i < SEQUENCER_MEASURES_NUM; i++)
            {
                const uint64_t time_until_send_notes = (TIMING_CLOCK_EVENT_BY_EIGHTH_NOTE - _alarms_since_eighth_note) * _get_tick_duration();
                const int64_t min_duration_before_midi_send = DETECT_MEASURE_MARBLES_DURATION + DETECT_EIGHTH_NOTE_MARBLES_DURATION;

                if (time_until_send_notes > min_duration_before_midi_send)
                {
                    measure_lock_event_t *event = _sequencer.get_measure_lock_event(i);
                    if (event != NULL)
                    {
                        marble_type_t measure_marble_types[SEQUENCER_EIGHTH_NOTE_BY_MEASURE_NUM * SEQUENCER_TRACKS_NUM];
                        _marble_detector.detect_measure_marbles(event->measure_index, measure_marble_types);
                        _sequencer.set_locked_measure_marble_types(event, measure_marble_types);
                    }
                }
                else
                {
                    if (_sequencer.has_locked_measure_events_pending())
                    {
                        schedule_read_newly_locked_measures();
                    }
                    vTaskDelay(pdMS_TO_TICKS(10));
                    break;
                }
            }
        }
    }
}


void MasterClock::handle_start_playing_event()
{
    start_playing();
}

void MasterClock::handle_stop_playing_event()
{
    stop_playing();
}

void MasterClock::handle_new_lock_event()
{
    schedule_read_newly_locked_measures();
}