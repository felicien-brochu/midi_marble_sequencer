#include "Sequencer.h"

#include <cstring>
#include <cstddef>
#include <cmath>
#include <esp_log.h>

static const char *TAG = "SEQUENCER";

Sequencer::Sequencer()
{
    for (size_t i = 0; i < SEQUENCER_PAGES_NUM; i++)
    {
        _played_pages[i] = (i == 0);
    }
    
    _edited_page_index = 0;

    _current_page_index = 0;
    _current_eighth_note_index = 0;

    _is_playing = false;
    _bpm = SEQUENCER_BPM_DEFAULT;
    for (size_t i = 0; i < SEQUENCER_BPM_SAMPLE_COUNT; i++)
    {
        _bpm_samples[i] = SEQUENCER_BPM_DEFAULT;
    }
    

    for (size_t i = 0; i < SEQUENCER_TRACKS_NUM; i++)
    {
        _tracks_enabled[i] = true;
    }

    for (size_t i = 0; i < SEQUENCER_MEASURES_NUM; i++)
    {
        _rotary_buttons_states[i] = ROTARY_BUTTON_PLAY;
        _measure_lock_events[i] = NULL;
    }
}

void Sequencer::set_sequencer_callback(sequencer_callback_t sequencer_callback, void *sequencer_callback_context)
{
    _sequencer_callback = sequencer_callback;
    _sequencer_callback_context = sequencer_callback_context;
}

controls_main_display_t Sequencer::get_controls_main_display()
{
    controls_main_display_t controls_main_display;
    controls_main_display.play_pause_led_enabled = _is_playing;
    memcpy(controls_main_display.tracks_led_enabled, _tracks_enabled, sizeof(_tracks_enabled));
    controls_main_display.bpm = !_is_playing ? _bpm : -1;

    return controls_main_display;
}

void Sequencer::handle_controls_main_event(const controls_main_value_t controls_main_value)
{
    _set_playing_from_play_pause_switch(controls_main_value.play_pause_switch_pushed);
    _set_bpm_from_potentiometer(controls_main_value.bpm_potentiometer_value);
    _set_tracks_enabled_from_push_buttons(controls_main_value.tracks_push_buttons);
    _set_measures_states_from_rotary_buttons(controls_main_value.rotary_buttons_states);
}

controls_pages_display_t Sequencer::get_controls_pages_display()
{
    controls_pages_display_t controls_pages_display;

    for (size_t i = 0; i < SEQUENCER_PAGES_NUM; i++)
    {
        led_display_state_t led_state = (i == _edited_page_index) ? LED_DISPLAY_STATE_ON : LED_DISPLAY_STATE_OFF;
        controls_pages_display.edited_pages_led_states[i] = led_state;
        
        
        led_state = LED_DISPLAY_STATE_OFF;

        if (_played_pages[i])
        {
            if (i == _current_page_index && _is_playing)
            {
                led_state = LED_DISPLAY_STATE_BLINK;
            }
            else
            {
                led_state = LED_DISPLAY_STATE_ON;
            }
        }
        controls_pages_display.played_pages_led_states[i] = led_state;
    }

    return controls_pages_display;
}

void Sequencer::handle_controls_pages_event(const controls_pages_value_t controls_pages_value)
{
    _set_played_pages_from_buttons(controls_pages_value.played_pages_buttons);
    _set_edited_pages_from_buttons(controls_pages_value.edited_pages_buttons);
}

bool Sequencer::is_playing()
{
    return _is_playing;
}

uint64_t Sequencer::get_eighth_note_duration()
{
    return (uint64_t) (60000000. / _bpm);
}

uint8_t Sequencer::get_current_eighth_note_index()
{
    return _current_eighth_note_index;
}

void Sequencer::next_eighth_note()
{
    SequencerPage &current_page = _pages[_current_page_index];
    
    if (!current_page.has_playable_eighth_notes_after(_current_eighth_note_index))
    {
        next_page();
        current_page = _pages[_current_page_index];
        _current_eighth_note_index = current_page.get_first_playable_eighth_note_index();
    }
    else
    {
        _current_eighth_note_index = current_page.get_first_playable_eighth_note_index_after(_current_eighth_note_index);
    }
}

void Sequencer::next_page()
{
    uint8_t next_page_index = _current_page_index + 1;
    for (; next_page_index < SEQUENCER_PAGES_NUM; next_page_index++)
    {
        if (_played_pages[next_page_index])
        {
            break;
        }
    }


    if (next_page_index >= SEQUENCER_PAGES_NUM)
    {
        next_page_index = _get_first_played_page_index();
    }
    
    _current_page_index = next_page_index;
}

void Sequencer::set_eighth_note_marble_types(marble_type_t *marble_types)
{
    SequencerPage &edited_page = _pages[_edited_page_index];
    edited_page.set_eighth_note_marble_types(_current_eighth_note_index, marble_types);
}

void Sequencer::get_current_eighth_note_marble_types(marble_type_t *marble_types)
{
    SequencerPage &current_page = _pages[_current_page_index];
    current_page.get_eighth_note_marble_types(_current_eighth_note_index, marble_types);
}

bool Sequencer::is_current_eighth_note_locked()
{
    SequencerPage &current_page = _pages[_current_page_index];
    return current_page.get_eighth_note_measure_state(_current_eighth_note_index) == MEASURE_STATE_LOCK;
}

measure_lock_event_t **Sequencer::get_measure_lock_events()
{
    return _measure_lock_events;
}

bool Sequencer::has_locked_measure_events_pending()
{
    for (uint8_t i = 0; i < SEQUENCER_MEASURES_NUM; i++)
    {
        if (_measure_lock_events[i] != NULL)
        {
            return true;
        }
    }
    return false;
}

void Sequencer::set_locked_measure_marble_types(measure_lock_event_t *event, marble_type_t *measure_marble_types)
{
    SequencerPage &page = _pages[event->page_index];
    page.set_measure_marble_types(event->measure_index, measure_marble_types);
    _measure_lock_events[event->measure_index] = NULL;
}

const bool *Sequencer::get_enabled_tracks()
{
    return _tracks_enabled;
}




void Sequencer::_set_playing_from_play_pause_switch(bool play_pause_switch_pushed)
{
    if (!_is_playing && play_pause_switch_pushed)
    {
        _start_playing();
    }
    else if (_is_playing && !play_pause_switch_pushed)
    {
        _stop_playing();
    }
}

void Sequencer::_start_playing()
{
    _current_page_index = _get_first_played_page_index();
    _current_eighth_note_index = _pages[_current_page_index].get_first_playable_eighth_note_index();
    _is_playing = true;
    _sequencer_callback(SEQUENCER_CB_START_PLAYING, NULL, _sequencer_callback_context);
}

void Sequencer::_stop_playing()
{
    _current_page_index = _get_first_played_page_index();
    _current_eighth_note_index = _pages[_current_page_index].get_first_playable_eighth_note_index();
    _is_playing = false;
    _sequencer_callback(SEQUENCER_CB_STOP_PLAYING, NULL, _sequencer_callback_context);
}

uint8_t Sequencer::_get_first_played_page_index()
{
    for (uint8_t i = 0; i < SEQUENCER_PAGES_NUM; i++)
    {
        if (_played_pages[i])
        {
            return i;
        }
    }
    
    return 0;
}

// Sets BPM from potentiometer value. Potentiometer value is 0 < value < 1.
void Sequencer::_set_bpm_from_potentiometer(const float potentiometer_value)
{
    // Use exponential function so that the ADC error is proportionally mapped to the final BPM value.
    const float base = (float) SEQUENCER_BPM_MAX / (float) SEQUENCER_BPM_MIN;
    float new_sample = SEQUENCER_BPM_MIN * pow(base, potentiometer_value);
    float new_bpm = 0;

    // smoothing BPM (removing noise)
    for (size_t i = SEQUENCER_BPM_SAMPLE_COUNT - 1; i > 0; i--)
    {
        _bpm_samples[i] = _bpm_samples[i - 1];
        // new_bpm += _bpm_samples[i] * (SEQUENCER_BPM_SAMPLE_COUNT - i);
        new_bpm += _bpm_samples[i];
    }
    _bpm_samples[0] = new_sample;
    // new_bpm += _bpm_samples[0] * SEQUENCER_BPM_SAMPLE_COUNT;
    new_bpm += _bpm_samples[0];
    
    // const float n_int_sum = (((float) SEQUENCER_BPM_SAMPLE_COUNT * ((float)SEQUENCER_BPM_SAMPLE_COUNT + 1.)) / 2.);
    // new_bpm = new_bpm / n_int_sum;
    new_bpm = new_bpm / SEQUENCER_BPM_SAMPLE_COUNT;

    _bpm = new_bpm;

    // ESP_LOGI(TAG, "sample: %f, New bpm: %f", new_sample, new_bpm);
}

void Sequencer::_set_tracks_enabled_from_push_buttons(const uint16_t push_buttons_events)
{
    uint16_t events = push_buttons_events;

    for (size_t i = SEQUENCER_TRACKS_NUM; i > 0; i--)
    {
        // has odd number of clicks pending
        if (events & 0x2)
        {
            _tracks_enabled[i - 1] = !_tracks_enabled[i - 1];
        }
        events = events >> 2;
    }
}


inline measure_state_t _rotary_button_state_to_measure_state(const rotary_button_state_t rotary_button_state)
{
    switch (rotary_button_state)
    {
        case ROTARY_BUTTON_SKIP:
            return MEASURE_STATE_SKIP;
        case ROTARY_BUTTON_PLAY:
            return MEASURE_STATE_PLAY;
        case ROTARY_BUTTON_LOCK:
            return MEASURE_STATE_LOCK;
        default:
            return MEASURE_STATE_UNKNOWN;
    }
}

void Sequencer::_set_measures_states_from_rotary_buttons(const rotary_button_state_t *rotary_buttons_states)
{
    measure_state_t new_page_measures_states[SEQUENCER_MEASURES_NUM];
    for (size_t i = 0; i < SEQUENCER_MEASURES_NUM; i++)
    {
        if (rotary_buttons_states[i] != ROTARY_BUTTON_UNKNOWN)
        {
            _rotary_buttons_states[i] = rotary_buttons_states[i];
            new_page_measures_states[i] = _rotary_button_state_to_measure_state(_rotary_buttons_states[i]);
        }
    }
    
    SequencerPage &edited_page = _pages[_edited_page_index];

    measure_state_t *page_measures_states = edited_page.get_measures_states();
    bool has_new_lock_state = false;

    for (size_t i = 0; i < SEQUENCER_MEASURES_NUM; i++)
    {
        if (_rotary_buttons_states[i] == ROTARY_BUTTON_LOCK)
        {
            if (page_measures_states[i] != MEASURE_STATE_LOCK  && page_measures_states[i] != MEASURE_STATE_SOFT_LOCK)
            {
                _register_new_measure_lock_event(i);
                has_new_lock_state = true;
            }
        }
        else
        {
            _delete_measure_lock_event(i);
        }
    }

    
    edited_page.set_measures_states(new_page_measures_states);
    
    if (has_new_lock_state)
    {
        _sequencer_callback(SEQUENCER_CB_NEW_LOCK_EVENT, NULL, _sequencer_callback_context);
    }
}

void Sequencer::_delete_measure_lock_event(size_t i)
{
    if (_measure_lock_events[i] != NULL)
    {
        free(_measure_lock_events[i]);
        _measure_lock_events[i] = NULL;
    }
}

void Sequencer::_register_new_measure_lock_event(size_t i)
{
    _delete_measure_lock_event(i);
    _measure_lock_events[i] = (measure_lock_event_t *)malloc(sizeof(measure_lock_event_t));
    _measure_lock_events[i]->page_index = _edited_page_index;
    _measure_lock_events[i]->measure_index = i;
}

void Sequencer::_set_played_pages_from_buttons(const uint16_t push_buttons_events)
{
    uint16_t events = push_buttons_events;

    bool played_pages_temp[SEQUENCER_PAGES_NUM];
    memcpy(played_pages_temp, _played_pages, SEQUENCER_PAGES_NUM * sizeof(bool));

    uint8_t num_pages_played = 0;

    for (size_t i = SEQUENCER_PAGES_NUM; i > 0; i--)
    {
        // has odd number of clicks pending
        if (events & 0x2)
        {
            // ESP_LOGI(TAG, "Played page btn clck [%d]", i - 1);
            played_pages_temp[i - 1] = !played_pages_temp[i - 1];
        }
        events = events >> 2;

        if (played_pages_temp[i - 1])
        {
            num_pages_played++;
        }
    }

    if (num_pages_played == 0)
    {
        for (size_t i = SEQUENCER_PAGES_NUM; i > 0; i--)
        {
            _played_pages[i] = (i == _current_page_index);
        }
    }
    else
    {
        memcpy(_played_pages, played_pages_temp, SEQUENCER_PAGES_NUM * sizeof(bool));
    }
}

void Sequencer::_set_edited_pages_from_buttons(const uint16_t push_buttons_events)
{
    uint16_t events = push_buttons_events;

    for (size_t i = SEQUENCER_PAGES_NUM; i > 0; i--)
    {
        // has odd number of clicks pending
        if (events & 0x2)
        {
            // ESP_LOGI(TAG, "Edited page btn clck [%d]", i - 1);
            _edited_page_index = i - 1;
            return;
        }
        events = events >> 2;
    }
}