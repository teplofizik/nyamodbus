//
// Nyamodbus library v1.0.0 emulator [input - contacts]
//

#define EMULATOR_INTERNAL

#include "emulator.h"
#include <string.h>

#define REG_COUNT   0x30

uint16_t holding[REG_COUNT];

// Read holding registers
//     id: index of register
//  value: where to write value 
// return: error code
static enum_nyamodbus_error read_holding(uint16_t id, uint16_t * value)
{
	if((id > 0) && (id <= REG_COUNT))
	{
		*value = holding[id - 1];
		
		return ERROR_OK;
	}
	else
		return ERROR_NO_DATAADDRESS;
}

// Write holding registers
//     id: index of register
//  value: where to write value 
// return: error code
static enum_nyamodbus_error write_holding(uint16_t id, uint16_t value)
{
	if((id > 0) && (id <= REG_COUNT))
	{
		holding[id - 1] = value;
		
		return ERROR_OK;
	}
	else
		return ERROR_NO_DATAADDRESS;
}

// Get device id string
// object: object id
// return: id string
const char * emu_readdeviceinfo(uint8_t object)
{
	switch(object)
	{
	case 0: // VendorName
		return "Nyamodbus";
	case 1: // ProductCode
		return "Holding example";
	case 2: // MajorMinorRevision
		return "v1.0.0";
	default: 
		return 0;
	}
}

// Slave id
static uint8_t slave_address = 0x11;
static str_nyamodbus_state state;

static const str_modbus_io io = {
	.send           = emu_slave_send,
	.receive        = emu_slave_receive
};

// Modbus slave state
static const str_nyamodbus_device modbus_slave = {
	.io =    &io,
	.state = &state
};

const str_nyamodbus_slave_device emuholding = {
	.device         = &modbus_slave,
	.address        = &slave_address,
	.readdeviceinfo = emu_readdeviceinfo,
	.readcontacts   = 0,
	.readanalog     = 0,
	.readcoils      = 0,
	.writecoil      = 0,
	.readholding    = read_holding,
	.writeholding   = write_holding,
};
