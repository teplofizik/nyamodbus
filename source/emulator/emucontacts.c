//
// Nyamodbus library v1.0.0 emulator [input - contacts]
//

#define EMULATOR_INTERNAL

#include "emulator.h"
#include <string.h>

// Read digital status
//     id: index of contact
// status: where to write status 
// return: error code
static enum_nyamodbus_error read_contacts(uint16_t id, bool * status)
{
	if((id > 0) && (id <= 24))
	{
		*status = (id % 0x04) == 0;
		
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
		return "Contacts example";
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

const str_nyamodbus_slave_device emucontacts = {
	.device         = &modbus_slave,
	.address        = &slave_address,
	.readdeviceinfo = emu_readdeviceinfo,
	.readcontacts   = read_contacts,
	.readanalog     = 0,
	.readcoils      = 0,
	.writecoil      = 0,
	.readholding    = 0,
	.writeholding   = 0,
};
