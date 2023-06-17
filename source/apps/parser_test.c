#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <emulator/emulator.h>

// Read holding command
void read_holding(void)
{
	uint8_t buffer[] = { 0x11, 0x03, 0x00, 0x6B, 0x00, 0x03, 0x76, 0x87 };
	emu_send(buffer, sizeof(buffer));
}

// Точка входа
int main(int argc, char *argv[])
{
	int i;
	
	emu_start(&emucontacts);
	usleep(10000);
	read_holding();
	usleep(10000);
	emu_stop();
	
	return 0;
}