#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <serial/serial.h>

static void master_error_cb(uint8_t slave, enum_nyamodbus_error error);
static void master_read_contacts_cb(uint8_t slave, uint16_t index, bool value);
static void master_read_coils_cb(uint8_t slave, uint16_t index, bool value);
static void master_read_inputs_cb(uint8_t slave, uint16_t index, uint16_t value);
static void master_read_holding_cb(uint8_t slave, uint16_t index, uint16_t value);

static str_nyamodbus_master_state master_state;

static const str_nyamodbus_master_device master = {
	.device        = &modbus_serial,
	.state         = &master_state,
	.on_error      = master_error_cb,
	.read_contacts = master_read_contacts_cb,
	.read_coils    = master_read_coils_cb,
	.read_inputs   = master_read_inputs_cb,
	.read_holding  = master_read_holding_cb
};

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

// Read coils command
void read_coils(void)
{
	nyamodbus_read_coils(&master, 0x20, 1, 10);
	usleep(10000);
}

// Read contacts command
void read_contacts(void)
{
	nyamodbus_read_contacts(&master, 0x20, 1, 10);
	usleep(10000);
}

// Read holding command
void read_holding(void)
{
	nyamodbus_read_holdings(&master, 0x20, 1, 10);
	usleep(10000);
}

// Read analog inputs command
void read_inputs(void)
{
	nyamodbus_read_inputs(&master, 0x20, 1, 10);
	usleep(10000);
}

// Write coil
void write_coil(void)
{
	
}

// Write holding
void write_holding(void)
{
	
}

// Write coils
void write_coils(void)
{
	
}

// Write holdings
void write_holdings(void)
{
	
}

void read_devid(void)
{
	
}

static void process_modbus(int argc, char *argv[], const char * dev)
{
	if(mbserial_master_start(dev, &master))
	{
		puts("Started");
		
		read_contacts();
		usleep(200000);
	
		mbserial_stop();
	}
	else
		puts("Cannot open tty device");
}

int main(int argc, char *argv[])
{
	int i;
	if(argc > 1)
	{
		process_modbus(argc - 2, &argv[2], argv[1]);
	}
	else
	{
		puts("Usage:");
		printf("%s <dev>\n", argv[0]);
		
	}
	return 0;
}