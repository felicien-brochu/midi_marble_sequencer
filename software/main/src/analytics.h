#include <inttypes.h>
#include "IRSensBoards.h"

void print_board_values(int *values_off, int *values_on, uint8_t ir_sens_on_board);

void statistics(int *values_off, int *values_on, IRSensBoards *ir_sens_boards, uint64_t print_every_seconds);

void distribution(int *values_off, int *values_on, uint8_t sensor_index, IRSensBoards *ir_sens_boards, uint64_t print_every_seconds);