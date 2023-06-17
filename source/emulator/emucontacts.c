//
// Nyamodbus library v1.0.0 emulator [input - contacts]
//

#include "emulator.h"

// Slave id
static uint8_t slave_address = 0x11;
static str_nyamodbus_state state;

static const str_nyamodbus_config config = {
	.address        = &slave_address,
	.send           = emu_send,
	.receive        = emu_receive,
	.readdeviceinfo = 0,
	.readcontacts   = 0,
	.readanalog     = 0,
	.readcoils      = 0,
	.writecoils     = 0,
	.readholding    = 0,
	.writeholding   = 0,
};

str_nyamodbus_device emucontacts = {
	.config = &config,
	.state = &state
};