#include <inttypes.h>
#include "IRSensBoard.h"

void print_values(int *values_off, int *values_on, uint8_t ir_sens_on_board);

void statistics(int *values_off, int *values_on, IRSensBoard *ir_sens_board, uint64_t print_every_seconds);

void distribution(int *values_off, int *values_on, uint8_t sensor_index, IRSensBoard *ir_sens_board, uint64_t print_every_seconds);