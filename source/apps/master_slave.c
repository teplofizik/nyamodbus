#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <emulator/emulator.h>
#include <emulator/emumaster.h>

// Read coils command
void read_coils(void)
{
	master_read_coils(0x11, 1, 10);
	usleep(10000);
}

// Read contacts command
void read_contacts(void)
{
	master_read_contacts(0x11, 1, 10);
	usleep(10000);
}

// Read holding command
void read_holding(void)
{
	master_read_holdings(0x11, 1, 10);
	usleep(10000);
}

// Read analog inputs command
void read_inputs(void)
{
	master_read_inputs(0x11, 1, 10);
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

int main(int argc, char *argv[])
{
	int i;
	
	master_start();
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
	read_devid();
	usleep(30000);
	emu_stop();
	master_stop();
	
	return 0;
}