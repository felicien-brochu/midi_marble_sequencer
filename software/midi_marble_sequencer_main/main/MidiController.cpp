#include "MidiController.h"
#include <class/midi/midi_device.h>
#include <esp_log.h>

#include "tinyusb.h"

static constexpr uint8_t MIDI_NOTE_OFF = 0x80;
static constexpr uint8_t MIDI_NOTE_ON = 0x90;
static constexpr uint8_t MIDI_TIMING_CLOCK = 0xF8;

// Interface counter
enum interface_count {
#if CFG_TUD_MIDI
    ITF_NUM_MIDI = 0,
    ITF_NUM_MIDI_STREAMING,
#endif
    ITF_COUNT
};

// USB Endpoint numbers
enum usb_endpoints {
    // Available USB Endpoints: 5 IN/OUT EPs and 1 IN EP
    EP_EMPTY = 0,
#if CFG_TUD_MIDI
    EPNUM_MIDI,
#endif
};

/** TinyUSB descriptors **/

#define TUSB_DESCRIPTOR_TOTAL_LEN (TUD_CONFIG_DESC_LEN + CFG_TUD_MIDI * TUD_MIDI_DESC_LEN)

/**
 * @brief String descriptor
 */
static const char *s_str_desc[5] = {
    // array of pointer to string descriptors
    (char[]){0x09, 0x04},    // 0: is supported language is English (0x0409)
    "Felicien Brochu",       // 1: Manufacturer
    "MIDI marble sequencer", // 2: Product
    "123456",                // 3: Serials, should use chip ID
    "MIDI marble sequencer", // 4: MIDI
};

/**
 * @brief Configuration descriptor
 *
 * This is a simple configuration descriptor that defines 1 configuration and a MIDI interface
 */
static const uint8_t s_midi_cfg_desc[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_COUNT, 0, TUSB_DESCRIPTOR_TOTAL_LEN, 0, 100),

    // Interface number, string index, EP Out & EP In address, EP size
    TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 4, EPNUM_MIDI, (0x80 | EPNUM_MIDI), 64),
};


#if (TUD_OPT_HIGH_SPEED)
/**
 * @brief High Speed configuration descriptor
 *
 * This is a simple configuration descriptor that defines 1 configuration and a MIDI interface
 */
static const uint8_t s_midi_hs_cfg_desc[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_COUNT, 0, TUSB_DESCRIPTOR_TOTAL_LEN, 0, 100),

    // Interface number, string index, EP Out & EP In address, EP size
    TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 4, EPNUM_MIDI, (0x80 | EPNUM_MIDI), 512),
};
#endif // TUD_OPT_HIGH_SPEED

MidiController::MidiController(Sequencer &sequencer) : _sequencer(sequencer)
{
    _notes_on_size = 0;

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
        .self_powered = true,
        .vbus_monitor_io = -1,
        };
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
}

static inline uint8_t _offset_midi_channel(uint8_t channel, uint8_t offset)
{
    // MIDI channels are 0-15.
    // For the requested mapping we expect base channels 0-7 and apply offset.
    if (channel <= (uint8_t)(0x0F - offset))
    {
        return (uint8_t)(channel + offset);
    }
    return 0x0F;
}

void MidiController::send_eighth_note_midi_notes()
{
    // Played page notes (channels 0-7)
    midi_note_t midi_notes[SEQUENCER_TRACKS_NUM * 2];
    marble_type_t marble_types_played[SEQUENCER_TRACKS_NUM];
    _sequencer.get_current_eighth_note_marble_types(marble_types_played);

    // Edited page notes (channels 8-15)
    midi_note_t midi_notes_edited[SEQUENCER_TRACKS_NUM];
    marble_type_t marble_types_edited[SEQUENCER_TRACKS_NUM];
    _sequencer.get_edited_eighth_note_marble_types(marble_types_edited);

    const bool *enabled_tracks = _sequencer.get_enabled_tracks();

    size_t num_notes = _midi_mapper.eighth_note_marble_types_to_midi_notes(midi_notes, marble_types_played, enabled_tracks);

    size_t num_notes_edited = _midi_mapper.eighth_note_marble_types_to_midi_notes(midi_notes_edited, marble_types_edited, enabled_tracks);
    for (size_t i = 0; i < num_notes_edited; i++)
    {
        midi_notes_edited[i].channel = _offset_midi_channel(midi_notes_edited[i].channel, 8);
        midi_notes[num_notes] = midi_notes_edited[i];
        num_notes++;
    }

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
            uint8_t note_on_plus_channel = (uint8_t) MIDI_NOTE_ON | midi_notes[i].channel;
            uint8_t note_on_message[3] = {note_on_plus_channel, midi_notes[i].note, 127};
            tud_midi_stream_write(midi_cable_num, note_on_message, 3);
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
            uint8_t note_off_plus_channel = (uint8_t) MIDI_NOTE_OFF | _notes_on[i].channel;
            uint8_t note_off_message[3] = {note_off_plus_channel, _notes_on[i].note, 0};
            tud_midi_stream_write(midi_cable_num, note_off_message, 3);
        }

        _notes_on_size = 0;
    }
}

void MidiController::send_timing_clock()
{
    if (tud_midi_mounted())
    {
        uint8_t midi_cable_num = 0;
        uint8_t timing_clock_message[1] = {MIDI_TIMING_CLOCK};
        tud_midi_stream_write(midi_cable_num, timing_clock_message, 1);
    }
}
