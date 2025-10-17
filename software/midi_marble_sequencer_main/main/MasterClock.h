#include <driver/gptimer.h>

#include "ControlBoardsController.h"
#include "MidiController.h"
#include "MarbleDetector.h"
#include "BeatsLEDSnake.h"

#define DEFAULT_EIGHTH_NOTE_DURATION 300000 //> Period of the timer in microseconds (=100BPM)
// #define DEFAULT_EIGHTH_NOTE_DURATION 2000000 //> Period of the timer in microseconds (=2s)
#define NUM_EIGHTH_NOTE (NUM_IR_SENS_BOARDS * 2) //> Number of eighth note on the sequencer

#define DETECT_EIGHTH_NOTE_MARBLES_DURATION 6000 //> Delay in us between MarbleDetector timer and MidiController timer to send eighth note notes

// #define DETECT_MEASURE_MARBLES_DURATION 8 * DETECT_EIGHTH_NOTE_MARBLES_DURATION
#define DETECT_MEASURE_MARBLES_DURATION 40000

#define TIMING_CLOCK_EVENT_BY_EIGHTH_NOTE 12

typedef enum {
    NO_REQ = 0,
    DETECT_MARBLES_REQ,
    SEND_MIDI_NOTES_REQ,
    SEND_TIMING_CLOCK_REQ
} timed_queue_request_t;

class MasterClock
{
public:
    MasterClock(Sequencer &sequencer);
    
    void start_playing();
    void stop_playing();
    void schedule_read_newly_locked_measures();
    void read_newly_locked_measures();
    
    BaseType_t handle_timing_clock_event();
    void execute_timed_queue_requests();
    void handle_start_playing_event();
    void handle_stop_playing_event();
    void handle_bpm_change_event();
    void handle_new_lock_event();
    
    
private:
    Sequencer &_sequencer;
    ControlBoardsController _control_boards_controller;
    
    MarbleDetector _marble_detector;
    MidiController _midi_controller;
    BeatsLEDSnake _led_snake;
    
    gptimer_handle_t _gptimer;
    QueueHandle_t _timed_queue;
    uint8_t _alarms_since_eighth_note;
    bool _eighth_note_marble_detected;
    TaskHandle_t _timed_queue_task_handle;
    QueueHandle_t _measure_lock_event_queue;
    TaskHandle_t _read_locked_measures_task_handle;
    
    void _start_timing_clock();
    void _detect_marbles();
    void _execute_detect_marbles_request();
    void _execute_send_midi_notes_request();
    void _execute_send_timing_clock_request();
    uint64_t _get_tick_duration();
};