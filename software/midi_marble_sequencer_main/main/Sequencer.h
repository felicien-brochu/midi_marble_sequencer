#pragma once

#include "sequencer_config.h"
#include "controls_main_common.h"
#include "SequencerPage.h"


#define SEQUENCER_BPM_MIN 20
#define SEQUENCER_BPM_MAX 600
#define SEQUENCER_BPM_DEFAULT 600
#define SEQUENCER_BPM_CHANGE_MIN 1


typedef enum {
    SEQUENCER_CB_START_PLAYING = 0,
    SEQUENCER_CB_STOP_PLAYING,
    SEQUENCER_CB_BPM_CHANGE,
    SEQUENCER_CB_NEW_LOCK_EVENT
} sequencer_callback_type_t;

typedef void (* sequencer_callback_t)(sequencer_callback_type_t callback_type, void *arg, void *context);


typedef struct {
    uint8_t page_index;
    uint8_t measure_index;
} measure_lock_event_t;


class Sequencer
{
public:
    Sequencer();

    void set_sequencer_callback(sequencer_callback_t sequencer_callback, void *sequencer_callback_context);
    controls_main_display_t get_controls_main_display();
    void handle_controls_main_event(const controls_main_value_t controls_main_value);
    bool is_playing();
    // Return eighth note duration in us.
    uint64_t get_eighth_note_duration();
    uint8_t get_current_eighth_note_index();
    void next_eighth_note();
    void next_page();
    void set_eighth_note_marble_types(marble_type_t *marble_types);
    void get_current_eighth_note_marble_types(marble_type_t *marble_types);
    bool is_current_eighth_note_locked();
    measure_lock_event_t **get_measure_lock_events();
    bool has_locked_measure_events_pending();
    void set_locked_measure_marble_types(measure_lock_event_t *event, marble_type_t *measure_marble_types);
    
private:
    sequencer_callback_t _sequencer_callback;
    void *_sequencer_callback_context;
    
    SequencerPage _pages[SEQUENCER_PAGES_NUM];
    bool _played_pages[SEQUENCER_PAGES_NUM];
    uint8_t _edited_page_index;

    uint8_t _current_page_index;
    uint8_t _current_eighth_note_index;
    
    
    bool _is_playing;
    float _bpm;
    bool _tracks_enabled[SEQUENCER_TRACKS_NUM];
    measure_state_t _measures_states[SEQUENCER_MEASURES_NUM];
    measure_lock_event_t *_measure_lock_events[SEQUENCER_MEASURES_NUM];

    
    void _start_playing();
    void _stop_playing();
    uint8_t _get_first_played_page_index();
    
    void _set_playing_from_play_pause_switch(bool play_pause_switch_pushed);
    void _set_bpm_from_potentiometer(const float potentiometer_value);
    void _set_tracks_enabled_from_push_buttons(const push_button_event_t *push_buttons_events);
    void _set_measures_states_from_rotary_buttons(const rotary_button_state_t *rotary_buttons_states);
};