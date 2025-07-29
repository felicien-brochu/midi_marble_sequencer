#include "MarbleDetector.h"
#include "MidiMapper.h"

#include "tinyusb.h"

#define NOTE_OFF 0x80
#define NOTE_ON  0x90


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
static const char* s_str_desc[5] = {
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

class MidiController
{
public:
    MidiController(MarbleDetector &marble_detector);
    void send_eighth_note_midi_notes();
    void send_notes_off();

    
private:
    
    MarbleDetector &_marble_detector;
    MidiMapper _midi_mapper;

    uint8_t _notes_on[NUM_IR_SENS_BOARDS * NUM_IR_SENS_BY_BOARD];
    size_t _notes_on_size;
    
    void _send_notes_on(uint8_t *midi_notes, size_t num_notes);
};