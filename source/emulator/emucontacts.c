//
// Nyamodbus library v1.0.0 emulator [input - contacts]
//

#define EMULATOR_INTERNAL

#include "emulator.h"

// Slave id
static uint8_t slave_address = 0x11;
static str_nyamodbus_state state;

static const str_nyamodbus_config config = {
	.address        = &slave_address,
	.send           = emu_send_internal,
	.receive        = emu_receive_internal,
	.readdeviceinfo = 0,
	.readcontacts   = 0,
	.readanalog     = 0,
	.readcoils      = 0,
	.writecoil      = 0,
	.readholding    = 0,
	.writeholding   = 0,
};

str_nyamodbus_device emucontacts = {
	.config = &config,
	.state = &state
};
