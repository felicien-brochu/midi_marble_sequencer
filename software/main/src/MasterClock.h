#include "MarbleDetector.h"
#include "MidiController.h"

#include <esp_timer.h>

#define DEFAULT_EIGHTH_NOTE_DURATION 300000 //> Period of the timer in microseconds (=100BPM)
// #define DEFAULT_EIGHTH_NOTE_DURATION 2000000 //> Period of the timer in microseconds (=2s)
#define NUM_EIGHTH_NOTE (NUM_IR_SENS_BOARDS * 2) //> Number of eighth note on the sequencer

#define DELAY_DETECT_MARBLES_THEN_SEND 15 //> Delay in ms between MarbleDetector timer and MidiController timer to send eighth note notes

class MasterClock
{
public:
    MasterClock();
    
    void start();
    void detect_marbles();
    void send_midi_notes();
    void on_marble_detector_timer_call_end();

private:
    MarbleDetector _marble_detector;
    MidiController _midi_controller;

    esp_timer_handle_t _marble_detector_timer_handle;
    esp_timer_handle_t _midi_controller_timer_handle;

    uint8_t _next_eighth_note_index;

    void _init_marble_detector_timer();
    void _init_midi_controller_timer();
};