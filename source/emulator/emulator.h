//
// Nyamodbus library v1.0.0 emulator [input - contacts]
//

#include <stdint.h>
#include <stdbool.h>

#ifndef _EMULATOR_H
#define _EMULATOR_H

	#include <nyamodbus/nyamodbus.h>

	// Receive emulator modbus data
	//   data: data to read
	//   size: size of buffer, size of readed data if result is true
	// return: true, if ok
	bool emu_receive(uint8_t * data, uint8_t * size);

	// Send emulator modbus data
	//   data: data to send
	//   size: size of data
	// return: true, if ok
	bool emu_send(const uint8_t * data, uint8_t size);

	// Contacts test
	extern str_nyamodbus_device emucontacts;

#endif