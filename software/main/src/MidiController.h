#include "MarbleDetector.h"

class MidiController
{
public:
    MidiController(MarbleDetector *marble_detector);
    void send_eighth_note_midi_notes();

private:

    MarbleDetector *_marble_detector;


};