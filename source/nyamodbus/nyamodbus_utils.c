//
// Nyamodbus library utils v1.1.0
//

#include "nyamodbus_utils.h"
#include <stddef.h>

// Get u16 value from packet
//   data: packet data
// offset: offset
// return: value
uint16_t get_u16_value(const uint8_t * data, uint8_t offset)
{
	return (((uint16_t)data[offset]) << 8) | data[offset + 1];
}

// Set u16 value to packet
//   data: packet data
// offset: offset
void set_u16_value(uint8_t * data, uint8_t offset, uint16_t value)
{
	data[offset]     = (value >> 8) & 0xFF;
	data[offset + 1] = value & 0xFF;
}

// Swap bytes in u16 value
uint16_t swap_u16(uint16_t val)
{
  return ((val & 0xFF) << 8) | ((val >> 8) & 0xFF);
}
