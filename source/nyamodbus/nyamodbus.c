//
// Nyamodbus library v1.0.0
//

#include "nyamodbus.h"
#include <string.h>

// Init modbus state
// device: device context
void nyamodbus_init(str_nyamodbus_device * device)
{
	// Init buffer...
	memset(&device->state, 0, sizeof(str_nyamodbus_state));
}

// Process byte
// device: device context
// byte: received byte
static void nyamodbus_processbyte(str_nyamodbus_device * device, uint8_t byte)
{
	
}

// Main processing cycle
// device: device context
void nyamodbus_main(str_nyamodbus_device * device)
{
	uint8_t buffer[100];
	uint8_t size = sizeof(buffer);
	
	// If something is available to receive...
	if(device->config->receive && device->config->receive(buffer, &size))
	{
		for(i = 0; i < size; i++)
		{
			nyamodbus_processbyte(device, byte);
		}
	}
}
