//
// Nyamodbus library v1.1.0 SLAVE
//

#include <stdint.h>
#include <stdbool.h>

#ifndef _NYAMODBUS_SLAVE_H
#define _NYAMODBUS_SLAVE_H

    #include "nyamodbus.h"

	// Driver configuration
	typedef struct {
		// Pointer to modbus struct
		const str_nyamodbus_device * device;
		
		// Pointer to slave address
		uint8_t *                    address;
		
		// Read device information
		nyamb_readdeviceinfo         readdeviceinfo;
		
		// Read contacts
		nyamb_readdigital            readcontacts;
		
		// Read analog inputs
		nyamb_readanalog             readanalog;
		
		// Read coil status
		nyamb_readdigital            readcoils;
		
		// Write coil status
		nyamb_writecoil              writecoil;
		
		// Read holding register
		nyamb_readanalog             readholding;
		
		// Write holding register
		nyamb_writeholding           writeholding;
	} str_nyamodbus_slave_device;
	
	// Init slave modbus state
	// device: device context
	void nyamodbus_slave_init(const str_nyamodbus_slave_device * device);
	
	// Init slave modbus state
	// device: device context
	void nyamodbus_slave_main(const str_nyamodbus_slave_device * device);

	// Trigger modbus timeout (parse received data)
	void nyamodbus_slave_timeout(const str_nyamodbus_slave_device * device);

	// Reset modbus state
	void nyamodbus_slave_reset(const str_nyamodbus_slave_device * device);

#endif