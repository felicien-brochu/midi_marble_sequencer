#include <cstdint>
#include <cstdlib>

#define CRC_POLY_16  0xA001
#define CRC_START_16 0x0000

uint16_t crc_16(const uint8_t *input_str, size_t num_bytes);
bool check_crc16(uint8_t *buffer, size_t num_bytes_with_crc);
void compute_crc16(uint8_t *buffer, size_t num_bytes_with_crc);