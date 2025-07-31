#include "crc.h"

#include <cstring>

static void init_crc16_tab();

static bool crc_tab16_initialized = false;
static uint16_t crc_tab16[256];

/*
 * uint16_t crc_16( const unsigned char *input_str, size_t num_bytes );
 *
 * The function crc_16() calculates the 16 bits CRC16 in one pass for a byte
 * string of which the beginning has been passed to the function. The number of
 * bytes to check is also a parameter. The number of the bytes in the string is
 * limited by the constant SIZE_MAX.
 */

uint16_t crc_16(const uint8_t *input_string, size_t num_bytes) {

	uint16_t crc;
	uint16_t tmp;
	uint16_t short_c;
	const uint8_t *ptr;
	size_t a;

	if (!crc_tab16_initialized) 
	{
		init_crc16_tab();
	}

	crc = CRC_START_16;
	ptr = input_string;

	if ( ptr != NULL ) for (a = 0; a < num_bytes; a++) {

		short_c = 0x00ff & (uint16_t) *ptr;
		tmp     =  crc       ^ short_c;
		crc     = (crc >> 8) ^ crc_tab16[ tmp & 0xff ];

		ptr++;
	}

	return crc;

}

bool check_crc16(uint8_t *buffer, size_t num_bytes_with_crc)
{
	uint16_t crc = crc_16(buffer, num_bytes_with_crc - sizeof(uint16_t));
	uint16_t crc_to_check;

	memcpy(&crc_to_check, buffer + num_bytes_with_crc - sizeof(uint16_t), sizeof(uint16_t));

	return crc_to_check == crc;
}

void compute_crc16(uint8_t *buffer, size_t num_bytes_with_crc)
{
	uint16_t crc = crc_16(buffer, num_bytes_with_crc - sizeof(uint16_t));
	memcpy(buffer + num_bytes_with_crc - sizeof(uint16_t), &crc, sizeof(uint16_t));
}


/*
 * static void init_crc16_tab( void );
 *
 * For optimal performance uses the CRC16 routine a lookup table with values
 * that can be used directly in the XOR arithmetic in the algorithm. This
 * lookup table is calculated by the init_crc16_tab() routine, the first time
 * the CRC function is called.
 */

static void init_crc16_tab() {

	uint16_t i;
	uint16_t j;
	uint16_t crc;
	uint16_t c;

	for (i=0; i<256; i++) {

		crc = 0;
		c   = i;

		for (j=0; j<8; j++) {

			if ( (crc ^ c) & 0x0001 ) crc = ( crc >> 1 ) ^ CRC_POLY_16;
			else                      crc =   crc >> 1;

			c = c >> 1;
		}

		crc_tab16[i] = crc;
	}

	crc_tab16_initialized = true;
}