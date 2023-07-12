//
// Nyamodbus library v1.0.0 emulator [master]
//

#define EMULATOR_INTERNAL

#include "emulator.h"
#include <nyamodbus/nyamodbus_master.h>
#include <nyamodbus/nyamodbus_utils.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

static str_nyamodbus_state state;
static str_nyamodbus_master_state master_state;

static const str_modbus_io io = {
	.send           = emu_master_send,
	.receive        = emu_master_receive
};

// Modbus slave state
static const str_nyamodbus_device modbus_master = {
	.io =    &io,
	.state = &state
};

static void master_error_cb(uint8_t slave, enum_nyamodbus_error error);
static void master_read_contacts_cb(uint8_t slave, uint16_t index, bool value);
static void master_read_coils_cb(uint8_t slave, uint16_t index, bool value);
static void master_read_inputs_cb(uint8_t slave, uint16_t index, uint16_t value);
static void master_read_holding_cb(uint8_t slave, uint16_t index, uint16_t value);

static const str_nyamodbus_master_device master = {
	.device        = &modbus_master,
	.state         = &master_state,
	.on_error      = master_error_cb,
	.read_contacts = master_read_contacts_cb,
	.read_coils    = master_read_coils_cb,
	.read_inputs   = master_read_inputs_cb,
	.read_holding  = master_read_holding_cb
};

// Timestamp to calc timeouts
static uint64_t                           emu_timestamp = 0;

// Thread control
static pthread_t                          emu_master_thread_id;

// Is emulator runnung                  
static bool                               emu_master_running = false;

// Init master
static void master_init(void)
{
	emu_timestamp = get_timestamp();
	
	nyamodbus_master_init(&master);
}

// Master loop
static void master_main(void)
{
	uint64_t time = get_timestamp();
	
	nyamodbus_master_tick(&master, time - emu_timestamp);
	nyamodbus_master_main(&master);
	
	emu_timestamp = time;
}

// Main emulator processing thread
static void * emu_master_thread(void * args)
{
	puts("Master (EMU) is started");
	master_init();
	
	while(emu_master_running)
	{
		master_main();
		usleep(10000);
	}
	
	puts("Master (EMU) is stopped");
	return 0;
}

// Start master device
void master_start(void)
{
	if(!emu_master_running)
	{
		pthread_attr_t attr;
		
		emu_master_running = true;
		
		pthread_attr_init(&attr);
		pthread_create(&emu_master_thread_id, &attr, emu_master_thread, 0);
	}
}

// Stop master device
void master_stop(void)
{
	emu_master_running = false;
	pthread_join(emu_master_thread_id, 0);
}

// On modbus error
static void master_error_cb(uint8_t slave, enum_nyamodbus_error error)
{
	printf("ERROR: %d", error);
}

// Read contacts callback
static void master_read_contacts_cb(uint8_t slave, uint16_t index, bool value)
{
	printf("CONTACT %03d: %d", index, value ? 1 : 0);
}

// Read coils callback
static void master_read_coils_cb(uint8_t slave, uint16_t index, bool value)
{
	printf("COIL %03d: %d", index, value ? 1 : 0);
}

// Read analog inputs
static void master_read_inputs_cb(uint8_t slave, uint16_t index, uint16_t value)
{
	printf("INPUT %03d: %04x", index, value);
}

// Read holding registers
static void master_read_holding_cb(uint8_t slave, uint16_t index, uint16_t value)
{
	printf("HOLDING %03d: %04x", index, value);
}


// Read coils
//  slave: address of slave device
//  index: coil id
//  count: coil count
void master_read_coils(uint8_t slave, uint16_t index, uint16_t count)
{
	nyamodbus_read_coils(&master, slave, index, count);
}

// Read contacts
//  slave: address of slave device
//  index: contact id
//  count: contact count
void master_read_contacts(uint8_t slave, uint16_t index, uint16_t count)
{
	nyamodbus_read_contacts(&master, slave, index, count);
}

// Read holding
//  slave: address of slave device
//  index: holding id
//  count: holding count
void master_read_holdings(uint8_t slave, uint16_t index, uint16_t count)
{
	nyamodbus_read_holdings(&master, slave, index, count);
}

// Read inputs
//  slave: address of slave device
//  index: input id
//  count: input count
void master_read_inputs(uint8_t slave, uint16_t index, uint16_t count)
{
	nyamodbus_read_inputs(&master, slave, index, count);
}

