//
// Nyamodbus config v1.1.0
//

#include <stdint.h>
#include <stdbool.h>

#ifndef _NYAMODBUS_CONFIG_H
#define _NYAMODBUS_CONFIG_H

	// Debug mode (0-3)
	#define DEBUG_OUTPUT                  3

	// Receive buffer size
	#define NYAMODBUS_BUFFER_SIZE         128

	// Send buffer size
	#define NYAMODBUS_OUTPUT_BUFFER_SIZE  128

	// Usecs to wait answer
	#define NYAMODBUS_PACKET_WAIT_TIMEOUT 4500

	// Usecs to wait start to answer
	#define NYAMODBUS_PACKET_START_TIMEOUT 30000
	
#endif
