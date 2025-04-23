#include "marble_type.h"
#include "IRSensBoard.h"
#include "IRSensBoardReaderOneShot.h"

class MarbleChangeDetector
{
public:
    MarbleChangeDetector();

    void update();

private:
    IRSensBoard _ir_sens_board;
    IRSensBoardReaderOneShot _board_reader;

    int *_values_on;
    int *_values_off;

    marble_type_t *_marble_types;
    uint16_t _marble_types_thresholds[16][WHITE_MARBLE + 1] = {
        {458, 1780, 2382, 3460},
        {611, 1437, 2516, 3478},
        {712, 1468, 2495, 3432},
        {697, 1344, 2197, 3272},
        {561, 1380, 2254, 3356},
        {577, 1288, 2262, 3321},
        {634, 1224, 2017, 3148},
        {702, 1222, 1916, 2907},
        {364, 1082, 1824, 3064},
        {442, 1159, 2045, 3228},
        {425, 1188, 1966, 3206},
        {344, 1074, 2015, 3134},
        {288, 1060, 1984, 3162},
        {314, 994, 1704, 2906},
        {287, 920, 1752, 2988},
        {304, 994, 1830, 3054}
    };

    void _print_sensor_marble_type(int sensor_channel);
};