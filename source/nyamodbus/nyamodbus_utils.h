//
// Nyamodbus library utils v1.1.0
//

#include <stdint.h>
#include <stdbool.h>

#ifndef _NYAMODBUS_UTILS_H
#define _NYAMODBUS_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

	// Get u16 value from packet
	//   data: packet data
	// offset: offset
	// return: value
	uint16_t get_u16_value(const uint8_t * data, uint8_t offset);

	// Set u16 value to packet
	//   data: packet data
	// offset: offset
	void set_u16_value(uint8_t * data, uint8_t offset, uint16_t value);

	// Swap bytes in u16 value
	uint16_t swap_u16(uint16_t val);

#ifdef __cplusplus
};
#endif

#endif
