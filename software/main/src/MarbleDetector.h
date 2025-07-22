#include "marble_type.h"
#include "IRSensBoards.h"
#include "IRSensReader.h"

class MarbleDetector
{
public:
    MarbleDetector();

    marble_type_t *detect_eighth_note_marbles(uint8_t eighth_note_index);
    marble_type_t *get_current_eighth_note_marbles();


private:
    IRSensBoards _ir_sens_boards;
    IRSensReader _ir_sens_reader;
    
    uint32_t _values_on[NUM_VALUE_BY_COLUMN];
    uint32_t _values_off[NUM_VALUE_BY_COLUMN];
    marble_type_t _eighth_note_marble_types[NUM_VALUE_BY_COLUMN];
    
    
    void _convert_eighth_note_values_to_marble_types(uint8_t eighth_note_index);
    const uint16_t *_get_thresholds_for_eighth_note(uint8_t eighth_note_index, uint8_t value_index);
    marble_type_t _get_marble_type(uint8_t value_index, const uint16_t *threshold);
};