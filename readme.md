# Embedded master/slave modbus C library

Applicable for any C projects on linux, stm32 etc.

## Integration

Examples of integration can be found in source/emulator directory: emumaster, emucontacts, emuholding.

At first, there is need to implement IO functions:
```typedef struct {
	// Send function
	nyamb_send           send;
	
	// Receive function
	nyamb_receive        receive;
	
	// Is sending
	nyamb_getstatus      is_txbusy;
} str_modbus_io;
```
The last is for MCU applications and not required on PC.

Example of bsp function declarations:
```
#include <stdint.h>

#ifndef _RS485_H
#define _RS485_H

	// Send modbus data (RS485 DE line will be HIGH while sending)
	//   data: data to send
	//   size: size of data
	// return: true, if ok
	bool rs485_send(const uint8_t * data, uint8_t size);

	// Receive modbus data
	//   data: data to read
	//   size: size of buffer, size of readed data if result is true
	// return: true, if ok
	bool rs485_receive(uint8_t * data, uint8_t * size);

	// Is TX still send data
	// return: true if TX busy
	bool rs485_isbusy(void);

#endif
```

Integration:
```
#include <nyamodbus/nyamodbus.h>

// Modbus IO interface
static const str_modbus_io io = {
	.send           = rs485_send,
	.receive        = rs485_receive
	.is_txbusy      = rs485_isbusy
};
```

Next, declare device variable:
```
static str_nyamodbus_state state;
static const str_nyamodbus_device modbus_device = {
	.io =    &io,
	.state = &state
};
```

Next steps are differs on master and slave.

## Modbus master

Fill master callbacks structure (only required for your application fields):
```
#include <nyamodbus/nyamodbus.h>
#include <nyamodbus/nyamodbus_master.h>

static void master_error_cb(uint8_t slave, enum_nyamodbus_error error);
static void master_read_holding_cb(uint8_t slave, uint16_t index, uint16_t value);

static const str_nyamodbus_master_device master = {
	.device        = &modbus_device,
	.state         = &master_state,
	.on_error      = master_error_cb,
	.read_contacts = 0,
	.read_coils    = 0,
	.read_inputs   = 0,
	.read_holding  = master_read_holding_cb
};
```

Common support function (init and event loop):
```
nyamodbus_master_init(&master);
```

In main while loop (or in thread):
```
nyamodbus_master_main(&master);
```

On while loop or in software timer there is need to indicate time for modbus library:
```
nyamodbus_master_tick(&master, usecs_from_last_call);
```

For example for linux thread:
```
nyamodbus_master_init(&master);
while(running)
{
	nyamodbus_master_main(&master);
	nyamodbus_master_tick(&master, 1000);
	
	usleep(1000);
}
```

Examples of callbacks functions:
```
// On modbus timeout mark a device as gone
static void master_error_cb(uint8_t slave, enum_nyamodbus_error error)
{
	if(error == ERROR_TIMEOUT)
	{
		if(slave == 1)
			device_1_timeout = 0;
		
		if(slave == 2)
			device_2_timeout = 0;
	}
}

static void master_read_holding_cb(uint8_t slave, uint16_t index, uint16_t value)
{
	if(slave == DEVICE_1)
	{
		device_1_timeout = DEFAULT_DEVICE_1_TIMEOUT;
		
		// Work with readed holdings from device 1
		// Index and value are provided for each readed register
		
	}
	else if(slave == ENC_BOARD)
	{
		device_2_timeout = DEFAULT_DEVICE_2_TIMEOUT;
		
		// Work with readed holdings from device 2
		// Index and value are provided for each readed register
	}
}
```

Check is library busy:
```
// Is master busy
bool mb485_is_busy(void)
{
	return nyamodbus_master_is_busy(&master);
}
```

Call for reading holdings:
```
nyamodbus_read_holdings(&master, DEVICE_2, REG_INDEX, REG_COUNT);
```

Call for write holdings:
```
uint16_t data[REG_COUNT];

// ... fill registers data ...

nyamodbus_write_holdings(&master, DEVICE_2, REG_INDEX, REG_COUNT, &data[0]);
```

## Modbus slave

Fill slave callbacks structure (only required for your application fields):
```
#include <nyamodbus/nyamodbus.h>
#include <nyamodbus/nyamodbus_slave.h>

static uint8_t slave_address = 1;

// Read holding registers
//     id: index of register
//  value: where to write value 
// return: error code
static enum_nyamodbus_error read_holding(uint16_t id, uint16_t * value);

// Write holding registers
//     id: index of register
//  value: where to write value 
// return: error code
static enum_nyamodbus_error write_holding(uint16_t id, uint16_t value);

// Get device id string
// object: object id
// return: id string
static const char * read_deviceinfo(uint8_t object);

static const str_nyamodbus_slave_device slave = {
	.device         = &modbus_device,
	.address        = &slave_address,
	.readdeviceinfo = read_deviceinfo,
	.readcontacts   = 0,
	.readanalog     = 0,
	.readcoils      = 0,
	.writecoil      = 0,
	.readholding    = read_holding,
	.writeholding   = write_holding
};
```

Callback read functions are only return requested value or return error code, callback write functions are only process writed data or return error code if data is not valid.

Examples of functions:
```
// Get device id string
// object: object id
// return: id string
static const char * read_deviceinfo(uint8_t object)
{
	pc_available_timeout = DEFAULT_PC_TIMEOUT;
	
	switch(object)
	{
	case 0: // VendorName
		return VENDOR_NAME;
	case 1: // ProductCode
		return PRODUCT_CODE;
	case 2: // MajorMinorRevision
		return PRODUCT_VERSION;
	default: 
		return 0;
	}
}

// Write holding registers
//     id: index of register
//  value: where to write value 
// return: error code
static enum_nyamodbus_error write_holding(uint16_t id, uint16_t value)
{
	if((id >= REGISTER_MIN_ADDRESS) && (id <= REGISTER_MAX_ADDRESS))
	{
		// Check values
		// return ERROR_INV_REQ_VALUE if value cannot be writed (for example writing 200 in percent regster with allowed values from 0 to 100)
		
		// Write allowed values to registers
	
		return ERROR_OK;
	}
	else
		return ERROR_NO_DATAADDRESS;
}

// Read holding registers
//     id: index of register
//  value: where to write value 
// return: error code
static enum_nyamodbus_error read_holding(uint16_t id, uint16_t * value)
{
	if((id >= REGISTER_MIN_ADDRESS) && (id <= REGISTER_MAX_ADDRESS))
	{
		// Write values to value
		// *value = ...;
		
		return ERROR_OK;
	}
	else
		return ERROR_NO_DATAADDRESS;
}
```

## 
