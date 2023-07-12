//
// Nyamodbus library v1.0.0 emulator [input - contacts]
//

#include <stdint.h>
#include <stdbool.h>

#ifndef _EMULATOR_H
#define _EMULATOR_H

	#include <nyamodbus/nyamodbus.h>
	#include <nyamodbus/nyamodbus_slave.h>

#ifdef EMULATOR_INTERNAL

	// Receive slave emulator modbus data
	//   data: data to read
	//   size: size of buffer, size of readed data if result is true
	// return: true, if ok
	bool emu_slave_receive(uint8_t * data, uint8_t * size);

	// Send slave emulator modbus data
	//   data: data to send
	//   size: size of data
	// return: true, if ok
	bool emu_slave_send(const uint8_t * data, uint8_t size);
	
	// Receive master emulator modbus data
	//   data: data to read
	//   size: size of buffer, size of readed data if result is true
	// return: true, if ok
	bool emu_master_receive(uint8_t * data, uint8_t * size);

	// Send master emulator modbus data
	//   data: data to send
	//   size: size of data
	// return: true, if ok
	bool emu_master_send(const uint8_t * data, uint8_t size);
	
#endif

	// Start device emulation
	// device: device info
	void emu_start(const str_nyamodbus_slave_device * device);

	// Stop device emulation
	void emu_stop();

	// Send data to emulator
	//   data: data to send
	//   size: size of data
	// return: true, if ok
	void emu_send(const uint8_t * data, uint8_t size);

	// Contacts test
	extern const str_nyamodbus_slave_device emucontacts;

#endif