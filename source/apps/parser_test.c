#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <emulator/emulator.h>

// Read coils command
void read_coils(void)
{
	uint8_t buffer[] = { 0x11, 0x01, 0x00, 0x13, 0x00, 0x25, 0x0E, 0x84 };
	emu_send(buffer, sizeof(buffer));
}

// Read contacts command
void read_contacts(void)
{
	uint8_t buffer[] = { 0x11, 0x02, 0x00, 0xC4, 0x00, 0x16, 0xBA, 0xA9 };
	emu_send(buffer, sizeof(buffer));
}

// Read holding command
void read_holding(void)
{
	uint8_t buffer[] = { 0x11, 0x03, 0x00, 0x6B, 0x00, 0x03, 0x76, 0x87 };
	emu_send(buffer, sizeof(buffer));
}

// Read analog inputs command
void read_inputs(void)
{
	uint8_t buffer[] = { 0x11, 0x04, 0x00, 0x08, 0x00, 0x01, 0xB2, 0x98 };
	emu_send(buffer, sizeof(buffer));
}

// Write coil
void write_coil(void)
{
	uint8_t buffer[] = { 0x11, 0x05, 0x00, 0xAC, 0xFF, 0x00, 0x4E, 0x8B };
	emu_send(buffer, sizeof(buffer));
}

// Write holding
void write_holding(void)
{
	uint8_t buffer[] = { 0x11, 0x06, 0x00, 0x01, 0x00, 0x03, 0x9A, 0x9B };
	emu_send(buffer, sizeof(buffer));
}

// Write coils
void write_coils(void)
{
	uint8_t buffer[] = { 0x11, 0x0F, 0x00, 0x13, 0x00, 0x0A, 0x02, 0xCD, 0x01, 0xBF, 0x0B };
	emu_send(buffer, sizeof(buffer));
}

// Write holdings
void write_holdings(void)
{
	uint8_t buffer[] = { 0x11, 0x10, 0x00, 0x01, 0x00, 0x02, 0x04, 0x00, 0x0A, 0x01, 0x02, 0xC6, 0xF0 };
	emu_send(buffer, sizeof(buffer));
}

int main(int argc, char *argv[])
{
	int i;
	
	emu_start(&emucontacts);
	usleep(10000);
	read_coils();
	read_contacts();
	read_holding();
	read_inputs();
	write_coil();
	write_holding();
	write_coils();
	write_holdings();
	usleep(30000);
	emu_stop();
	
	return 0;
}