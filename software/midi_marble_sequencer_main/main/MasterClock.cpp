#include "MasterClock.h"

#include <esp_err.h>
#include <freertos/freeRTOS.h>
#include <esp_log.h>

static const char *TAG = "MASTER_CLOCK";

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
    case SEQUENCER_CB_BPM_CHANGE:
        master_clock->handle_bpm_change_event();
        break;
    }
}

MasterClock::MasterClock(Sequencer &sequencer) : _sequencer(sequencer), _control_boards_controller(sequencer), _midi_controller(_marble_detector)
{
    _sequencer.set_sequencer_callback(sequencer_callback, this);
    _control_boards_controller.start_control_boards_task();
}

static void _marble_detector_timer_callback(void *master_clock_arg)
{
    MasterClock *master_clock = (MasterClock *)master_clock_arg;
    master_clock->detect_marbles();
    master_clock->on_marble_detector_timer_call_end();
}

static void _midi_controller_timer_callback(void *master_clock_arg)
{
    MasterClock *master_clock = (MasterClock *) master_clock_arg;
    master_clock->on_midi_controller_timer_call_start();
    master_clock->schedule_next_eighth_note_timers();
    master_clock->send_midi_notes();
    master_clock->on_midi_controller_timer_call_end();
}

void MasterClock::_init_marble_detector_timer()
{
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &_marble_detector_timer_callback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "MarbleDetector",
        .skip_unhandled_events = false
    };

    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &_marble_detector_timer_handle));
}

void MasterClock::_init_midi_controller_timer()
{
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &_midi_controller_timer_callback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "MidiController",
        .skip_unhandled_events = false
    };

    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &_midi_controller_timer_handle));
}

void MasterClock::schedule_next_eighth_note_timers()
{
    uint64_t eighth_note_duration = _sequencer.get_eighth_note_duration();
    int64_t current_time = esp_timer_get_time();
    int64_t midi_controller_timer_timeout = _last_midi_controller_timer_expiration + eighth_note_duration - current_time;

    if (midi_controller_timer_timeout < DELAY_DETECT_MARBLES_THEN_SEND)
    {
        midi_controller_timer_timeout = DELAY_DETECT_MARBLES_THEN_SEND;
    }
    
    if (esp_timer_is_active(_marble_detector_timer_handle) && esp_timer_is_active(_midi_controller_timer_handle))
    {
        ESP_ERROR_CHECK(esp_timer_restart(_marble_detector_timer_handle, midi_controller_timer_timeout - DELAY_DETECT_MARBLES_THEN_SEND));
        ESP_ERROR_CHECK(esp_timer_restart(_midi_controller_timer_handle, midi_controller_timer_timeout));
    }
    else if (!esp_timer_is_active(_marble_detector_timer_handle) && !esp_timer_is_active(_midi_controller_timer_handle))
    {
        ESP_ERROR_CHECK(esp_timer_delete(_marble_detector_timer_handle));
        ESP_ERROR_CHECK(esp_timer_delete(_midi_controller_timer_handle));
        _init_marble_detector_timer();
        _init_midi_controller_timer();

        ESP_ERROR_CHECK(esp_timer_start_once(_marble_detector_timer_handle, midi_controller_timer_timeout - DELAY_DETECT_MARBLES_THEN_SEND));
        ESP_ERROR_CHECK(esp_timer_start_once(_midi_controller_timer_handle, midi_controller_timer_timeout));
    }
}

void MasterClock::start_playing()
{
    _last_midi_controller_timer_expiration = 0;
    
    _init_marble_detector_timer();
    _init_midi_controller_timer();
    ESP_ERROR_CHECK(esp_timer_start_once(_marble_detector_timer_handle, 0));
    ESP_ERROR_CHECK(esp_timer_start_once(_midi_controller_timer_handle, DELAY_DETECT_MARBLES_THEN_SEND));
    
    _led_snake.set_eighth_note_index(_sequencer.get_current_eighth_note_index());
    _led_snake.set_enabled(true);
}

void MasterClock::stop_playing()
{
    if (esp_timer_is_active(_marble_detector_timer_handle))
    {
        ESP_ERROR_CHECK(esp_timer_stop(_marble_detector_timer_handle));
    }
    if (esp_timer_is_active(_midi_controller_timer_handle))
    {
        ESP_ERROR_CHECK(esp_timer_stop(_midi_controller_timer_handle));
    }

    ESP_ERROR_CHECK(esp_timer_delete(_marble_detector_timer_handle));
    ESP_ERROR_CHECK(esp_timer_delete(_midi_controller_timer_handle));

    _midi_controller.send_notes_off();

    _led_snake.set_enabled(false);
    _led_snake.update();
}

void MasterClock::update_bpm()
{
    if (_sequencer.is_playing())
    {
        schedule_next_eighth_note_timers();
    }
}

void MasterClock::detect_marbles()
{
    marble_type_t *marble_types = _marble_detector.detect_eighth_note_marbles(_sequencer.get_current_eighth_note_index());

    for (size_t i = 0; i < NUM_VALUE_BY_COLUMN; i++)
    {
        if (marble_types[i] != NO_MARBLE)
        {
            ESP_LOGI(TAG, "%d[%d]: %s", _sequencer.get_current_eighth_note_index(), i, marble_type_to_string(marble_types[i]));
        }
    }
}

void MasterClock::send_midi_notes()
{
    _midi_controller.send_eighth_note_midi_notes();
}

void MasterClock::on_marble_detector_timer_call_end()
{
    _led_snake.set_eighth_note_index(_sequencer.get_current_eighth_note_index());
    _sequencer.next_eighth_note();
}

void MasterClock::on_midi_controller_timer_call_start()
{
    _last_midi_controller_timer_expiration = esp_timer_get_time();
}

void MasterClock::on_midi_controller_timer_call_end()
{
    _led_snake.update();
}

void MasterClock::handle_start_playing_event()
{
    start_playing();
}

void MasterClock::handle_stop_playing_event()
{
    stop_playing();
}

void MasterClock::handle_bpm_change_event()
{
    update_bpm();
}