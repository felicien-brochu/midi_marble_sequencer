#include "MidiMapper.h"
#include "IRSensBoards.h"
#include "sequencer_config.h"

/* MIDI Note Numbers for General MIDI Percussion Key Map
Key# Note Drum Sound 
35 B0 Acoustic Bass Drum 
36 C1 Bass Drum 1     *
37 C#1 Side Stick 
38 D1 Acoustic Snare  *
39 Eb1 Hand Clap      *
40 E1 Electric Snare 
41 F1 Low Floor Tom 
42 F#1 Closed Hi Hat  *
43 G1 High Floor Tom 
44 Ab1 Pedal Hi-Hat 
45 A1 Low Tom 
46 Bb1 Open Hi-Hat 
47 B1 Low-Mid Tom 
48 C2 Hi Mid Tom 
49 C#2 Crash Cymbal 1 
50 D2 High Tom 
51 Eb2 Ride Cymbal 1 
52 E2 Chinese Cymbal 
53 F2 Ride Bell       *
54 F#2 Tambourine 
55 G2 Splash Cymbal 
56 Ab2 Cowbell 
57 A2 Crash Cymbal 2 
58 Bb2 Vibraslap
59 B2 Ride Cymbal 2
60 C3 Hi Bongo
61 C#3 Low Bongo
62 D3 Mute Hi Conga
63 Eb3 Open Hi Conga
64 E3 Low Conga
65 F3 High Timbale
66 F#3 Low Timbale
67 G3 High Agogo
68 Ab3 Low Agogo
69 A3 Cabasa
70 Bb3 Maracas
71 B3 Short Whistle
72 C4 Long Whistle
73 C#4 Short Guiro
74 D4 Long Guiro
75 Eb4 Claves
76 E4 Hi Wood Block
77 F4 Low Wood Block
78 F#4 Mute Cuica
79 G4 Open Cuica
80 Ab4 Mute Triangle
81 A4 Open Triangle
*/



static const uint8_t marble_types_melodic_mapping[] = {
    36, 38, 39, 41, 43, 45, 46,
    48, 50, 51, 53, 55, 57, 58,
    60, 62, 63, 65, 67, 69, 70,
    72, 74, 75, 77, 79, 81, 82,
    84, 86, 87, 89, 91, 93, 94,
    96, 98, 99, 101, 103, 105, 106,
    108, 110, 111, 113, 115, 117, 118,
};

MidiMapper::MidiMapper()
{
}

size_t MidiMapper::eighth_note_marble_types_to_midi_notes(midi_note_t *midi_notes, const marble_type_t *marble_types, const bool *enabled_tracks)
{
    size_t num_midi_notes = 0;
    for (uint8_t track_index = 0; track_index < SEQUENCER_TRACKS_NUM; track_index++)
    {
        if (enabled_tracks[track_index])
        {
            marble_type_t marble_type = marble_types[track_index];
            if (marble_type != NO_MARBLE)
            {
                midi_notes[num_midi_notes] = _eighth_note_marble_type_to_midi_note_default(track_index, marble_type);
                num_midi_notes++;
            }
        }
    }
    
    return num_midi_notes;
}


midi_note_t MidiMapper::_eighth_note_marble_type_to_midi_note_default(uint8_t track_index, marble_type_t marble_type)
{
    static const uint8_t notes[] = {60, 63, 65, 67, 70};
    midi_note_t midi_note = {
        .note = notes[marble_type - 1],
        .channel = track_index,
    };

    return midi_note;
}


midi_note_t _live_1_drum(marble_type_t marble_type)
{
    static const uint8_t notes[] = {36, 38, 42, 39, 53};
    uint8_t marble_type_index = (marble_type - 1);

    midi_note_t midi_note = {
        .note = notes[marble_type_index],
        .channel = 0
    };

    return midi_note;
}

midi_note_t _live_1_bass(uint8_t track_index, marble_type_t marble_type)
{
    static const uint8_t bass_notes[] = {
        34, 36, 41, 42, 43,
        46, 48, 53, 54, 55,
        58, 60, 65, 66, 67
    };

    uint8_t marble_type_index = (marble_type - 1);
    midi_note_t midi_note = {
        .note = bass_notes[marble_type_index * 3 + track_index],
        .channel = 1
    };

    return midi_note;
}

midi_note_t _live_1_melodic(uint8_t track_index, marble_type_t marble_type)
{
    static const uint8_t melodic_notes[] = {
        48, 51, 53, 55, 58,
        60, 63, 65, 67, 70,
        72, 75, 77, 79, 82
    };

    uint8_t marble_type_index = (marble_type - 1);
    midi_note_t midi_note = {
        .note = melodic_notes[marble_type_index * 3 + track_index],
        .channel = 2
    };

    return midi_note;
}


midi_note_t MidiMapper::_eighth_note_marble_type_to_midi_note_live_1(uint8_t track_index, marble_type_t marble_type)
{
    if (track_index >= 6)
    {
        return _live_1_drum(marble_type);
    }
    else if (track_index >= 3)
    {
        return _live_1_bass(2 - (track_index - 3), marble_type);
    }
    else
    {
        return _live_1_melodic(2 - track_index, marble_type);
    }
}

midi_note_t MidiMapper::_eighth_note_marble_type_to_midi_note_melodic(uint8_t track_index, marble_type_t marble_type)
{
    uint8_t track_index_inv = SEQUENCER_TRACKS_NUM - track_index - 1;
    uint8_t marble_type_index = (marble_type - 1);

    midi_note_t midi_note = {
        .note = marble_types_melodic_mapping[marble_type_index * SEQUENCER_TRACKS_NUM + track_index_inv],
        .channel = 0
    };

    return midi_note;
}

midi_note_t MidiMapper::_eighth_note_marble_type_to_midi_note_boom_box(uint8_t track_index, marble_type_t marble_type)
{
    static const uint8_t notes[] = {53, 60, 67};
    uint8_t marble_type_index = (marble_type - 1);

    midi_note_t midi_note = {
        .note = notes[marble_type_index],
        .channel = track_index
    };

    return midi_note;
}
