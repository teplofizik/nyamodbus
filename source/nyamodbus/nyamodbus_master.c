//
// Nyamodbus library master v1.1.0
//

#include "nyamodbus_master.h"
#include "nyamodbus_utils.h"
#include <string.h>
#include <stdio.h>

static void nyamodbus_master_on_valid_packet(void * context, const uint8_t * data, uint16_t size);
static void nyamodbus_master_on_timeout(void * context);
static void nyamodbus_master_on_data(void * context);

const str_nyamodbus_driver master_driver = {
	.on_data            = nyamodbus_master_on_data,
	.on_valid_packet    = nyamodbus_master_on_valid_packet,
	.on_invalid_packet  = nyamodbus_master_on_timeout,
	.on_timeout         = nyamodbus_master_on_timeout,
};

// Init modbus state
// device: device context
void nyamodbus_master_init(const str_nyamodbus_master_device * device)
{
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
	puts("nyamodbus_master_init");
	
	if(!device->state)
		puts("Empty pointer to state!");
	
	if(!device->device)
		puts("Empty pointer to device!");
#endif

	memset(device->state, 0, sizeof(str_nyamodbus_master_state));

	nyamodbus_init(device->device);
}

// Function to parse modbus packet
//   data: data
//   size: size of data
static void nyamodbus_master_on_valid_packet(void * context, const uint8_t * data, uint16_t size)
{
	str_nyamodbus_master_device * device = (str_nyamodbus_master_device *)context;
	
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
	puts("Valid response!");
#endif


}

static void nyamodbus_master_on_timeout(void * context)
{
	str_nyamodbus_master_device * device = (str_nyamodbus_master_device *)context;
	
	// Generate timeout error
	if (device->on_error) 
		device->on_error(device->state->command[0], ERROR_TIMEOUT);
	
	memset(device->state, 0, sizeof(str_nyamodbus_master_state));
}

// Any data received
static void nyamodbus_master_on_data(void * context)
{
	str_nyamodbus_master_device * device = (str_nyamodbus_master_device *)context;
	
	nyamodbus_start_timeout(device->device);
	nyamodbus_reset_timeout(device->device);
}

// Main processing cycle
// device: device context
void nyamodbus_master_main(const str_nyamodbus_master_device * device)
{
	nyamodbus_main(device->device, &master_driver, (void*)device);
}

// Reset modbus state
void nyamodbus_master_reset(const str_nyamodbus_master_device * device)
{
	memset(device->state, 0, sizeof(str_nyamodbus_master_state));
			
	nyamodbus_reset(device->device);
}

// Trigger modbus timeout (parse received data)
//  device: device context
//   usecs: useconds after last call
// context: driver context
void nyamodbus_master_tick(const str_nyamodbus_master_device * device, uint32_t usecs)
{
	nyamodbus_tick(device->device, &master_driver, (void*)device, usecs);
}

// Read coils
// device: device context
//  slave: address of slave device
//  index: coil id
//  count: coil count
void nyamodbus_read_coils(const str_nyamodbus_master_device * device, uint8_t slave, uint16_t index, uint16_t count)
{
	uint8_t * buffer = &device->state->command[0];
	
	buffer[0] = slave;
	buffer[1] = FUNCTION_READ_COIL;
	set_u16_value(buffer, 2, index);
	set_u16_value(buffer, 4, count);
	
	nyamodbus_send_packet(device->device, buffer, 6);
	if(slave != 255) nyamodbus_start_timeout(device->device);
}

// Read contacts
// device: device context
//  slave: address of slave device
//  index: contact id
//  count: contact count
void nyamodbus_read_contacts(const str_nyamodbus_master_device * device, uint8_t slave, uint16_t index, uint16_t count)
{
	uint8_t * buffer = &device->state->command[0];
	
	buffer[0] = slave;
	buffer[1] = FUNCTION_READ_CONTACTS;
	set_u16_value(buffer, 2, index);
	set_u16_value(buffer, 4, count);
	
	nyamodbus_send_packet(device->device, buffer, 6);
	if(slave != 255) nyamodbus_start_timeout(device->device);
}

// Read holding
// device: device context
//  slave: address of slave device
//  index: holding id
//  count: holding count
void nyamodbus_read_holdings(const str_nyamodbus_master_device * device, uint8_t slave, uint16_t index, uint16_t count)
{
	uint8_t * buffer = &device->state->command[0];
	
	buffer[0] = slave;
	buffer[1] = FUNCTION_READ_HOLDING;
	set_u16_value(buffer, 2, index);
	set_u16_value(buffer, 4, count);
	
	nyamodbus_send_packet(device->device, buffer, 6);
	if(slave != 255) nyamodbus_start_timeout(device->device);
}

// Read inputs
// device: device context
//  slave: address of slave device
//  index: input id
//  count: input count
void nyamodbus_read_inputs(const str_nyamodbus_master_device * device, uint8_t slave, uint16_t index, uint16_t count)
{
	uint8_t * buffer = &device->state->command[0];
	
	buffer[0] = slave;
	buffer[1] = FUNCTION_READ_INPUTS;
	set_u16_value(buffer, 2, index);
	set_u16_value(buffer, 4, count);
	
	nyamodbus_send_packet(device->device, buffer, 6);
	if(slave != 255) nyamodbus_start_timeout(device->device);
}
