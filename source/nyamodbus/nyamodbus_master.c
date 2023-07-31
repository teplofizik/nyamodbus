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

// Parse "read contacts" response
//        device: device context
//  request_data: request data
//  request_size: request data size
// response_data: response data
// response_size: response data size
static void nyamodbus_master_parse_read_contacts(str_nyamodbus_master_device * device, const uint8_t * request_data, uint16_t request_size, const uint8_t * response_data, uint16_t response_size)
{
	// Check request info...
	uint16_t address = get_u16_value(request_data, 2);
	uint16_t count   = get_u16_value(request_data, 4);
	uint8_t  bytes = (count + 7) / 8;
	uint8_t  slave = response_data[0];
	
	if(bytes == response_data[2]) // expected payload size
	{
		int i;
		for(i = 0; i < count; i++)
		{
			uint8_t bit = i & 0x7;
			uint8_t byte = i / 8;
			uint16_t reg = address + i;
			uint8_t mask = response_data[3 + byte];
			
			bool value = (mask & (1 << bit));
			device->read_contacts(slave, reg, value);
		}
	}
}

// Parse "read coils" response
//        device: device context
//  request_data: request data
//  request_size: request data size
// response_data: response data
// response_size: response data size
static void nyamodbus_master_parse_read_coils(str_nyamodbus_master_device * device, const uint8_t * request_data, uint16_t request_size, const uint8_t * response_data, uint16_t response_size)
{
	// Check request info...
	uint16_t address = get_u16_value(request_data, 2);
	uint16_t count   = get_u16_value(request_data, 4);
	uint8_t  bytes = (count + 7) / 8;
	uint8_t  slave = response_data[0];
	
	if(bytes == response_data[2]) // expected payload size
	{
		int i;
		for(i = 0; i < count; i++)
		{
			uint8_t bit = i & 0x7;
			uint8_t byte = i / 8;
			uint16_t reg = address + i;
			uint8_t mask = response_data[3 + byte];
			
			bool value = (mask & (1 << bit));
			device->read_coils(slave, reg, value);
		}
	}
}

// Parse "read holding" response
//        device: device context
//  request_data: request data
//  request_size: request data size
// response_data: response data
// response_size: response data size
static void nyamodbus_master_parse_read_holding(str_nyamodbus_master_device * device, const uint8_t * request_data, uint16_t request_size, const uint8_t * response_data, uint16_t response_size)
{
	// Check request info...
	uint16_t address = get_u16_value(request_data, 2);
	uint16_t count   = get_u16_value(request_data, 4);
	uint8_t  bytes = count * 2;
	uint8_t  slave = response_data[0];
	
	if(bytes == response_data[2]) // expected payload size
	{
		int i;
		for(i = 0; i < count; i++)
		{
			uint16_t byte = i * 2ul;
			uint16_t  reg = address + i;
			
			uint16_t value = ((uint16_t)response_data[byte] << 8) | response_data[byte + 1];
			device->read_holding(slave, reg, value);
		}
	}
}

// Parse "read holding" response
//        device: device context
//  request_data: request data
//  request_size: request data size
// response_data: response data
// response_size: response data size
static void nyamodbus_master_parse_read_inputs(str_nyamodbus_master_device * device, const uint8_t * request_data, uint16_t request_size, const uint8_t * response_data, uint16_t response_size)
{
	// Check request info...
	uint16_t address = get_u16_value(request_data, 2);
	uint16_t count   = get_u16_value(request_data, 4);
	uint8_t  bytes = count * 2;
	uint8_t  slave = response_data[0];
	
	if(bytes == response_data[2]) // expected payload size
	{
		int i;
		for(i = 0; i < count; i++)
		{
			uint16_t byte = i * 2ul;
			uint16_t  reg = address + i;
			
			uint16_t value = ((uint16_t)response_data[byte] << 8) | response_data[byte + 1];
			device->read_inputs(slave, reg, value);
		}
	}
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

	if(device->on_response)
	{
		if(device->on_response(data[0], data, size))
			return;
	}
	
	// Check data...
	if (((data[0] == device->state->command[0]) && (device->state->command[0] != 0xFF)) && // Slave ok
		((data[1] & 0x7F) == device->state->command[1])) // Func ok
	{
		// TODO: check response size!
		
		// Parse reesponse...
		if((data[1] & 0x80) != 0)
		{
			// Parse error code
		}
		else
		{
			// Normal response
			switch(data[1]) // Parse by function...
			{
				case FUNCTION_READ_CONTACTS:
					if(device->read_contacts)
						nyamodbus_master_parse_read_contacts(device, &device->state->command[0], device->state->size, data, size);
					break;
					
				case FUNCTION_READ_COIL:
					if(device->read_coils)
						nyamodbus_master_parse_read_coils(device, &device->state->command[0], device->state->size, data, size);
					break;
					
				case FUNCTION_READ_HOLDING:
					if(device->read_holding)
						nyamodbus_master_parse_read_holding(device, &device->state->command[0], device->state->size, data, size);
					break;
					
				case FUNCTION_READ_INPUTS:
					if(device->read_inputs)
						nyamodbus_master_parse_read_inputs(device, &device->state->command[0], device->state->size, data, size);
					break;
			}
		}
	}
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

// Send packet
// device: device context
//   data: data to send
//   size: data size
void nyamodbus_master_send_packet(const str_nyamodbus_master_device * device, const uint8_t * data, uint8_t size)
{
	if(size <= sizeof(device->state->command))
	{
		uint8_t slave = data[0];
		
		device->state->size = size;
		memcpy(&device->state->command[0], data, size);
		
		nyamodbus_send_packet(device->device, data, size);
		if(slave != 255) nyamodbus_start_timeout(device->device);
	}
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
	uint8_t buffer[6];
	
	buffer[0] = slave;
	buffer[1] = FUNCTION_READ_COIL;
	set_u16_value(buffer, 2, index);
	set_u16_value(buffer, 4, count);
	
	nyamodbus_master_send_packet(device, buffer, 6);
}

// Read contacts
// device: device context
//  slave: address of slave device
//  index: contact id
//  count: contact count
void nyamodbus_read_contacts(const str_nyamodbus_master_device * device, uint8_t slave, uint16_t index, uint16_t count)
{
	uint8_t buffer[6];
	
	buffer[0] = slave;
	buffer[1] = FUNCTION_READ_CONTACTS;
	set_u16_value(buffer, 2, index);
	set_u16_value(buffer, 4, count);
	
	nyamodbus_master_send_packet(device, buffer, 6);
}

// Read holding
// device: device context
//  slave: address of slave device
//  index: holding id
//  count: holding count
void nyamodbus_read_holdings(const str_nyamodbus_master_device * device, uint8_t slave, uint16_t index, uint16_t count)
{
	uint8_t buffer[6];
	
	buffer[0] = slave;
	buffer[1] = FUNCTION_READ_HOLDING;
	set_u16_value(buffer, 2, index);
	set_u16_value(buffer, 4, count);
	
	nyamodbus_master_send_packet(device, buffer, 6);
}

// Read inputs
// device: device context
//  slave: address of slave device
//  index: input id
//  count: input count
void nyamodbus_read_inputs(const str_nyamodbus_master_device * device, uint8_t slave, uint16_t index, uint16_t count)
{
	uint8_t buffer[6];
	
	buffer[0] = slave;
	buffer[1] = FUNCTION_READ_INPUTS;
	set_u16_value(buffer, 2, index);
	set_u16_value(buffer, 4, count);
	
	nyamodbus_master_send_packet(device, buffer, 6);
}
