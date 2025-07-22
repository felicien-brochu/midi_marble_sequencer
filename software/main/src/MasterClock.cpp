#include <esp_err.h>
#include <freertos/freeRTOS.h>

#include "MasterClock.h"

MasterClock::MasterClock() : _marble_detector(), _midi_controller(&_marble_detector)
{
    _next_eighth_note_index = 0;

    _init_marble_detector_timer();
}

static void _marble_detector_timer_callback(void *master_clock_arg)
{
    MasterClock *master_clock = (MasterClock *)master_clock_arg;
    master_clock->detect_marbles();
    master_clock->on_marble_detector_timer_call_end();
}

static void _midi_controller_timer_callback(void *master_clock_arg)
{
    MasterClock *master_clock = (MasterClock *)master_clock_arg;
    master_clock->send_midi_notes();
}

void MasterClock::_init_marble_detector_timer()
{
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &_marble_detector_timer_callback,
        .arg = this,
        .name = "MarbleDetector"
    };

    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &_marble_detector_timer_handle));
}

void MasterClock::_init_midi_controller_timer()
{
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &_midi_controller_timer_callback,
        .arg = this,
        .name = "MidiController"
    };

    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &_midi_controller_timer_handle));
}

void MasterClock::start()
{
    ESP_ERROR_CHECK(esp_timer_start_periodic(_marble_detector_timer_handle, DEFAULT_EIGHTH_NOTE_DURATION));

    vTaskDelay(pdMS_TO_TICKS(DELAY_DETECT_MARBLES_THEN_SEND));
    ESP_ERROR_CHECK(esp_timer_start_periodic(_midi_controller_timer_handle, DEFAULT_EIGHTH_NOTE_DURATION));
}

void MasterClock::detect_marbles()
{
    marble_type_t *marble_types = _marble_detector.detect_eighth_note_marbles(_next_eighth_note_index);

    printf("8th note: %d\n", _next_eighth_note_index);
    for (size_t i = 0; i < NUM_VALUE_BY_COLUMN; i++)
    {
        printf("%s\n", marble_type_to_string(marble_types[i]));
    }
    printf("\n\n");
}

void MasterClock::send_midi_notes()
{
    _midi_controller.send_eighth_note_midi_notes();
}

void MasterClock::on_marble_detector_timer_call_end()
{
    if (_next_eighth_note_index >= NUM_EIGHTH_NOTE)
    {
        _next_eighth_note_index = 0;
    }
    else
    {
        _next_eighth_note_index++;
    }
}