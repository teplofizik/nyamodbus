//
// Nyamodbus library master v1.1.0
//

#include "nyamodbus_master.h"
#include <string.h>

static void nyamodbus_master_on_valid_packet(void * context, const uint8_t * data, uint16_t size);
static void nyamodbus_master_on_timeout(void * context);

const str_nyamodbus_driver master_driver = {
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
	if (device->onerror) 
		device->onerror(ERROR_TIMEOUT);
	
	memset(device->state, 0, sizeof(str_nyamodbus_master_state));
}

// Main processing cycle
// device: device context
void nyamodbus_master_main(const str_nyamodbus_master_device * device)
{
	nyamodbus_init(device->device);
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

// Reset modbus timeout (while receiving data)
// device: device context
void nyamodbus_master_reset_timeout(const str_nyamodbus_master_device * device)
{
	nyamodbus_reset_timeout(device->device);
}
