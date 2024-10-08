//
// Nyamodbus library master v1.1.0
//

#include <stdint.h>
#include <stdbool.h>

#ifndef _NYAMODBUS_MASTER_H
#define _NYAMODBUS_MASTER_H

#include "nyamodbus.h"
	
#ifdef __cplusplus
extern "C" {
#endif

	// Digital read (coils, contacts)
	typedef void (*nyam_master_digital_read)(uint8_t slave, uint16_t index, bool value);

	// Analog read (holding, input)
	typedef void (*nyam_master_analog_read)(uint8_t slave, uint16_t index, uint16_t value);
	
	// Device info respinse handler
	// index: index of string
	//  info: information string
	typedef void (*nyam_master_device_info)(uint8_t slave, uint8_t index, const char * info);
	
	// Valid response handler
	// return: true, if processing completed
	typedef bool (*nyam_response)(uint8_t slave, const uint8_t * data, uint16_t size);
	
	// Error code handler
	typedef void (*nyam_request_error)(uint8_t slave,enum_nyamodbus_error error);
	
	// Master state
	typedef struct
	{
		// Send buffer
		uint8_t    command[NYAMODBUS_OUTPUT_BUFFER_SIZE];
		// Send buffer size
		uint8_t    size;
	} str_nyamodbus_master_state;
	
	// Master device context
	typedef struct 
	{
		// Pointer to modbus struct
		const str_nyamodbus_device * device;
		
		// Pointer to modbus master state
		str_nyamodbus_master_state * state;
		
		// Respone received
		nyam_response                on_response;
		
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

	// Reset modbus state
	// device: device context
	void nyamodbus_master_reset(const str_nyamodbus_master_device * device);

	// Is master busy
	// device: device context
	bool nyamodbus_master_is_busy(const str_nyamodbus_master_device * device);

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

	// Write holding
	// device: device context
	//  slave: address of slave device
	//  index: holding id
	//  count: holding count
	//   data: register data [count]
	void nyamodbus_write_holdings(const str_nyamodbus_master_device * device, uint8_t slave, uint16_t index, uint16_t count, uint16_t * data);

#ifdef __cplusplus
};
#endif


#endif
