#include <esp_timer.h>

#include "ControlBoardsController.h"
#include "MidiController.h"
#include "MarbleDetector.h"
#include "BeatsLEDSnake.h"

#define DEFAULT_EIGHTH_NOTE_DURATION 300000 //> Period of the timer in microseconds (=100BPM)
// #define DEFAULT_EIGHTH_NOTE_DURATION 2000000 //> Period of the timer in microseconds (=2s)
#define NUM_EIGHTH_NOTE (NUM_IR_SENS_BOARDS * 2) //> Number of eighth note on the sequencer

#define DELAY_DETECT_MARBLES_THEN_SEND 30000 //> Delay in us between MarbleDetector timer and MidiController timer to send eighth note notes

class MasterClock
{
public:
    MasterClock(Sequencer &sequencer);
    
    void start_playing();
    void stop_playing();
    void update_bpm();
    void detect_marbles();
    void send_midi_notes();
    void schedule_next_eighth_note_timers();

    void on_marble_detector_timer_call_end();
    void on_midi_controller_timer_call_start();
    void on_midi_controller_timer_call_end();

    void handle_start_playing_event();
    void handle_stop_playing_event();
    void handle_bpm_change_event();

    
private:
    Sequencer &_sequencer;
    ControlBoardsController _control_boards_controller;

    MarbleDetector _marble_detector;
    MidiController _midi_controller;
    BeatsLEDSnake _led_snake;


    int64_t _last_midi_controller_timer_expiration;
    esp_timer_handle_t _marble_detector_timer_handle;
    esp_timer_handle_t _midi_controller_timer_handle;
    
    void _init_marble_detector_timer();
    void _init_midi_controller_timer();
};