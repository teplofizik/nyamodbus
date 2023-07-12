//
// Nyamodbus library master v1.1.0
//

#include <stdint.h>
#include <stdbool.h>

#ifndef _NYAMODBUS_MASTER_H
#define _NYAMODBUS_MASTER_H

	#include "nyamodbus.h"
	
	// Digital read (coils, contacts)
	typedef void (*nyam_master_digital_read)(uint16_t index, bool value);

	// Analog read (holding, input)
	typedef void (*nyam_master_analog_read)(uint16_t index, uint16_t value);
	
	// Device id
	// index: index of string
	//  info: information string
	typedef void (*nyam_master_device_info)(uint8_t index, const char * info);
	
	// Error code
	typedef void (*nyam_request_error)(enum_nyamodbus_error error);
	
	// Master state
	typedef struct
	{
		// Send buffer
		uint8_t    command[NYAMODBUS_OUTPUT_BUFFER_SIZE];
	} str_nyamodbus_master_state;
	
	// Master device context
	typedef struct 
	{
		// Pointer to modbus struct
		const str_nyamodbus_device * device;
		
		// Pointer to modbus master state
		str_nyamodbus_master_state * device;
		
		// Error code
		nyam_request_error           on_error;
		
		// Contacts read handler
		nyam_master_digital_read     read_contacts;
		
		// Coils read handler
		nyam_master_digital_read     read_coils;
		
		// Inputs read handler
		nyam_master_analog_read      read_inputs;
		
		// Holding read handler
		nyam_master_analog_read      read_holding;
	} str_nyamodbus_master_device;
	
	// Init modbus state
	// device: device context
	void nyamodbus_master_init(const str_nyamodbus_master_device * device);

	// Main processing cycle
	// device: device context
	void nyamodbus_master_main(const str_nyamodbus_master_device * device);
	
	// Trigger modbus timeout (parse received data)
	//  device: device context
	//   usecs: useconds after last call
	// context: driver context
	void nyamodbus_master_tick(const str_nyamodbus_master_device * device, uint32_t usecs);

	// Reset modbus timeout (while receiving data)
	// device: device context
	void nyamodbus_master_reset_timeout(const str_nyamodbus_master_device * device);

	// Reset modbus state
	// device: device context
	void nyamodbus_master_reset(const str_nyamodbus_master_device * device);

	// Is master busy
	// device: device context
	bool nyamodbus_is_busy(const str_nyamodbus_master_device * device)

	// Read coils
	// device: device context
	//  slave: address of slave device
	//  index: coil id
	//  count: coil count
	void nyamodbus_read_coils(const str_nyamodbus_master_device * device, uint8_t slave, uint16_t index, uint16_t count);

	// Read contacts
	// device: device context
	//  slave: address of slave device
	//  index: contact id
	//  count: contact count
	void nyamodbus_read_contacts(const str_nyamodbus_master_device * device, uint8_t slave, uint16_t index, uint16_t count);

	// Read holding
	// device: device context
	//  slave: address of slave device
	//  index: holding id
	//  count: holding count
	void nyamodbus_read_holdings(const str_nyamodbus_master_device * device, uint8_t slave, uint16_t index, uint16_t count);

	// Read inputs
	// device: device context
	//  slave: address of slave device
	//  index: input id
	//  count: input count
	void nyamodbus_read_inputs(const str_nyamodbus_master_device * device, uint8_t slave, uint16_t index, uint16_t count);

#endif
