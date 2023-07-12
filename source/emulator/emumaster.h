//
// Nyamodbus library v1.0.0 emulator [input - contacts]
//

#include <stdint.h>
#include <stdbool.h>

#ifndef _EMULATOR_MASTER_H
#define _EMULATOR_MASTER_H

	// Start master device
	void master_start(void);

	// Stop master device
	void master_stop(void);

	// Read coils
	//  slave: address of slave device
	//  index: coil id
	//  count: coil count
	void master_read_coils(uint8_t slave, uint16_t index, uint16_t count);

	// Read contacts
	//  slave: address of slave device
	//  index: contact id
	//  count: contact count
	void master_read_contacts(uint8_t slave, uint16_t index, uint16_t count);

	// Read holding
	//  slave: address of slave device
	//  index: holding id
	//  count: holding count
	void master_read_holdings(uint8_t slave, uint16_t index, uint16_t count);

	// Read inputs
	//  slave: address of slave device
	//  index: input id
	//  count: input count
	void master_read_inputs(uint8_t slave, uint16_t index, uint16_t count);


#endif
