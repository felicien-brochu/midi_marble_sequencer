#include "MidiController.h"
#include <class/midi/midi_device.h>


MidiController::MidiController(Sequencer &sequencer) : _sequencer(sequencer)
{
    tinyusb_config_t const tusb_cfg = {
        .device_descriptor = NULL, // If device_descriptor is NULL, tinyusb_driver_install() will use Kconfig
        .string_descriptor = s_str_desc,
        .string_descriptor_count = sizeof(s_str_desc) / sizeof(s_str_desc[0]),
        .external_phy = false,
    #if (TUD_OPT_HIGH_SPEED)
        .fs_configuration_descriptor = s_midi_cfg_desc, // HID configuration descriptor for full-speed and high-speed are the same
        .hs_configuration_descriptor = s_midi_hs_cfg_desc,
        .qualifier_descriptor = NULL,
    #else
        .configuration_descriptor = s_midi_cfg_desc,
    #endif // TUD_OPT_HIGH_SPEED
        };
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
}

void MidiController::send_eighth_note_midi_notes()
{
    midi_note_t midi_notes[SEQUENCER_TRACKS_NUM];
    marble_type_t marble_types[SEQUENCER_TRACKS_NUM];
    _sequencer.get_current_eighth_note_marble_types(marble_types);

    size_t num_notes = _midi_mapper.eighth_note_marble_types_to_midi_notes(midi_notes, marble_types);

    send_notes_off();
    _send_notes_on(midi_notes, num_notes);
}

void MidiController::_send_notes_on(midi_note_t *midi_notes, size_t num_notes)
{
    if (tud_midi_mounted())
    {
        uint8_t midi_cable_num = 0;

        for (size_t i = 0; i < num_notes; i++)
        {
            // printf("Send note_on: %d\n", midi_notes[i]);
            uint8_t note_on_plus_channel = (uint8_t) NOTE_ON | midi_notes[i].channel;
            uint8_t note_on[3] = {note_on_plus_channel, midi_notes[i].note, 127};
            tud_midi_stream_write(midi_cable_num, note_on, 3);
            _notes_on[i] = midi_notes[i];
        }

        _notes_on_size = num_notes;
    }
}

void MidiController::send_notes_off()
{
    if (tud_midi_mounted())
    {
        uint8_t midi_cable_num = 0;

        for (size_t i = 0; i < _notes_on_size; i++)
        {
            uint8_t note_off_plus_channel = (uint8_t)NOTE_OFF | _notes_on[i].channel;
            uint8_t note_off[3] = {note_off_plus_channel, _notes_on[i].note, 0};
            tud_midi_stream_write(midi_cable_num, note_off, 3);
        }

        _notes_on_size = 0;
    }
}
