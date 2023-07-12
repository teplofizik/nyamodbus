//
// Nyamodbus library v1.0.0 serial
//

#ifndef MODBUS_SERIAL_H
#define MODBUS_SERIAL_H

	#include <nyamodbus/nyamodbus_master.h>
	
	// Modbus slave state
	extern const str_nyamodbus_device modbus_master;

	// Start serial modbus master on dtty evice
	//    dev: path to tty device
	// device: device config
	// return: true, if started
	bool mbserial_start(const char * dev, const str_nyamodbus_master_device * device);
	
	// Stop serial modbus device
	void mbserial_stop(void);

#endif
