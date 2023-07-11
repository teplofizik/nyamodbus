//
// Nyamodbus library v1.0.0 emulator [input - contacts]
//

#define EMULATOR_INTERNAL

#include "emulator.h"
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

// Temp buffer
uint8_t buffer[100];
uint8_t buffer_index = 0;
uint8_t buffer_size = 0;

// Emulator device
static const str_nyamodbus_slave_device * emu_device = 0;

// Thread control
static pthread_t                          emu_thread_id;

// Thread mutex                         
static pthread_mutex_t                    emu_data_mutex;

// Is emulator runnung                  
static bool                               emu_running = false;

// Main emulator processing thread
static void * emu_thread(void * args)
{
	puts("Emulator is started");
	nyamodbus_slave_init(emu_device);
	while(emu_running)
	{
		nyamodbus_slave_main(emu_device);
		usleep(10000);
	}
	
	puts("Emulator is stopped");
	return 0;
}

// Start device emulation
// device: device info
void emu_start(const str_nyamodbus_slave_device * device)
{
	if(!emu_running)
	{
		pthread_attr_t attr;
		
		emu_device = device;
		emu_running = true;
		
		pthread_attr_init(&attr);
		pthread_create(&emu_thread_id, &attr, emu_thread, 0);
	}
}

// Stop device emulation
void emu_stop()
{
	emu_running = false;
	pthread_join(emu_thread_id, 0);
}

// Is emulator buffer busy
bool emu_is_tx_busy(void)
{
	bool busy = false;
	pthread_mutex_lock(&emu_data_mutex);
	busy = (buffer_size != 0);
	pthread_mutex_unlock(&emu_data_mutex);
	
	return busy;
}

// Wait tx to free
static void remu_wait(void)
{
	while(emu_is_tx_busy())
	{
		usleep(1000);
	}
}

// Send data to emulator
//   data: data to send
//   size: size of data
// return: true, if ok
void emu_send(const uint8_t * data, uint8_t size)
{
	remu_wait();
	if(size <= sizeof(buffer))
	{
		pthread_mutex_lock(&emu_data_mutex);
		memcpy(&buffer[0], data, size);
		
		buffer_size = size;
		pthread_mutex_unlock(&emu_data_mutex);
	}
	else
	{
		printf("Message is too big for internal buffer: %u > %lu\n", size, sizeof(buffer));
	}
	
	remu_wait();
	nyamodbus_slave_timeout(emu_device);
	// else todo
}

// Send emulator modbus data
//   data: data to send
//   size: size of data
// return: true, if ok
bool emu_send_internal(const uint8_t * data, uint8_t size)
{
	pthread_mutex_lock(&emu_data_mutex);
	printf("Response: ");
	for(int i = 0; i < size; i++)
	{
		printf("%02x ", data[i]);
		
		if((i % 0x10) == 0x0f) puts("");
	}
	puts("");
	pthread_mutex_unlock(&emu_data_mutex);
	
	return true;
}

// Receive emulator modbus data
//   data: data to read
//   size: size of buffer, size of readed data if result is true
// return: true, if ok
bool emu_receive_internal(uint8_t * data, uint8_t * size)
{
	pthread_mutex_lock(&emu_data_mutex);
	uint8_t tosend = buffer_size - buffer_index;
	if(tosend > 0)
	{
		if(*size < tosend) 
			tosend = *size;
		
		memcpy(data, &buffer[buffer_index], tosend);
		*size = tosend;
		buffer_index += tosend;
		
		if(buffer_index == buffer_size)
		{
			buffer_index = 0;
			buffer_size = 0;
		}
	}
	pthread_mutex_unlock(&emu_data_mutex);
	return (tosend > 0);
}
