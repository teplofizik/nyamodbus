//
// Nyamodbus library v1.0.0 serial
//

#ifndef MODBUS_SERIAL_H
#define MODBUS_SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

	#include <nyamodbus/nyamodbus_master.h>
	#include <nyamodbus/nyamodbus_slave.h>
	
	// Modbus slave state
	extern const str_nyamodbus_device modbus_serial;

	// Start serial modbus master on tty service
	//    dev: path to tty device
	// device: device config
	// return: true, if started
	bool mbserial_master_start(const char * dev, const str_nyamodbus_master_device * device);

	// Start serial modbus master on tty service
	//    dev: path to tty device
	// device: device config
	// return: true, if started
	bool mbserial_slave_start(const char * dev, const str_nyamodbus_slave_device * device);

	// Stop serial modbus device
	void mbserial_stop(void);

#ifdef __cplusplus
};
#endif

#endif
