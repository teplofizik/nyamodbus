//
// Nyamodbus library v1.0.0 emulator [input - contacts]
//

#include <stdint.h>
#include <stdbool.h>

#ifndef _EMULATOR_H
#define _EMULATOR_H

	#include <nyamodbus/nyamodbus.h>

#ifdef EMULATOR_INTERNAL

	// Receive emulator modbus data
	//   data: data to read
	//   size: size of buffer, size of readed data if result is true
	// return: true, if ok
	bool emu_receive_internal(uint8_t * data, uint8_t * size);

	// Send emulator modbus data
	//   data: data to send
	//   size: size of data
	// return: true, if ok
	bool emu_send_internal(const uint8_t * data, uint8_t size);
	
#endif

	// Start device emulation
	// device: device info
	void emu_start(str_nyamodbus_device * device);

	// Stop device emulation
	void emu_stop();

	// Send data to emulator
	//   data: data to send
	//   size: size of data
	// return: true, if ok
	void emu_send(const uint8_t * data, uint8_t size);

	// Contacts test
	extern str_nyamodbus_device emucontacts;

#endif