#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <emulator/emulator.h>

uint8_t buffer[] = { 0x11, 0x03, 0x00, 0x6B, 0x00, 0x03, 0x76, 0x87 };
uint8_t buffer_index = 0;
uint8_t buffer_size = sizeof(buffer);

// Send emulator modbus data
//   data: data to send
//   size: size of data
// return: true, if ok
bool emu_send(const uint8_t * data, uint8_t size)
{
	printf("Response: ");
	for(int i = 0; i < size; i++)
	{
		printf("%02x ", data[i]);
		
		if((i % 0x10) == 0x0f) puts("");
	}
	
	puts("");
	
	return true;
}

// Receive emulator modbus data
//   data: data to read
//   size: size of buffer, size of readed data if result is true
// return: true, if ok
bool emu_receive(uint8_t * data, uint8_t * size)
{
	uint8_t tosend = buffer_size - buffer_index;
	if(*size < tosend) 
		tosend = *size;
	
	memcpy(data, &buffer[buffer_index], tosend);
	*size = tosend;
	buffer_index += tosend;
	
	return true;
}

// Точка входа
int main(int argc, char *argv[])
{
	int i;
	str_nyamodbus_device * device = &emucontacts;
	nyamodbus_init(device);
	
	for(i = 0; i < 100; i++)
	{
		nyamodbus_main(device);
	}
	
	return 0;
}