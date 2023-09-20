//
// Nyamodbus library v1.0.0 emulator [input - contacts]
//

#define EMULATOR_INTERNAL

#include "emulator.h"
#include <nyamodbus/nyamodbus_utils.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

typedef struct
{
	uint8_t data[100];
	uint8_t index;
	uint8_t size;
} str_emu_buffer;

// Master->slave buffer
str_emu_buffer master_slave;

// Slave->master buffer
str_emu_buffer slave_master;

// Emulator device
static const str_nyamodbus_slave_device * emu_device = 0;

// Thread control
static pthread_t                          emu_thread_id;

// Thread mutex                         
static pthread_mutex_t                    emu_data_mutex;

// Is emulator runnung                  
static bool                               emu_running = false;

// Get current timestamp
uint64_t get_timestamp(void)
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
	
	return tv.tv_sec*1000000ULL + tv.tv_usec;
}

// Reset buffer to default state
static void buffer_reset(str_emu_buffer * buffer)
{
	pthread_mutex_lock(&emu_data_mutex);
	memset(buffer, 0, sizeof(str_emu_buffer));
	pthread_mutex_unlock(&emu_data_mutex);
}

// Receive emulator modbus data
// buffer: buffer to work with
//   data: data to read
//   size: size of buffer, size of readed data if result is true
// return: true, if ok
static bool emu_receive_buffer(str_emu_buffer * buffer, uint8_t * data, uint8_t * size)
{
	uint8_t tosend;
	pthread_mutex_lock(&emu_data_mutex);
	
	tosend = buffer->size - buffer->index;
	if(tosend > 0)
	{
		if(*size < tosend) 
			tosend = *size;
		
		memcpy(data, &buffer->data[buffer->index], tosend);
		*size = tosend;
		buffer->index += tosend;
		
		if(buffer->index == buffer->size)
		{
			buffer->index = 0;
			buffer->size = 0;
		}
	}
	else
		*size = 0;
	
	pthread_mutex_unlock(&emu_data_mutex);
}

// Is emulator buffer busy
static bool emu_is_buffer_busy(str_emu_buffer * buffer)
{
	bool busy = false;
	pthread_mutex_lock(&emu_data_mutex);
	busy = (buffer->size != 0);
	pthread_mutex_unlock(&emu_data_mutex);
	
	return busy;
}

// Wait tx to free
static void emu_wait_buffer(str_emu_buffer * buffer)
{
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
	bool busy = false;
#endif
	
	while(emu_is_buffer_busy(buffer))
	{
		usleep(1000);
		
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
		printf(".");
		busy = true;
#endif
	}
	
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
	if(busy) puts(".");
#endif
}

// Send data to buffer
// buffer: buffer to work with
//   data: data to send
//   size: size of data
// return: true, if ok
void emu_send_buffer(str_emu_buffer * buffer, const uint8_t * data, uint8_t size)
{
	emu_wait_buffer(buffer);
	if(size <= sizeof(buffer->data))
	{
		pthread_mutex_lock(&emu_data_mutex);
		memcpy(&buffer->data[0], data, size);
		buffer->size = size;
		
		pthread_mutex_unlock(&emu_data_mutex);
	}
	else
	{
		printf("Message is too big for internal buffer: %u > %lu\n", size, sizeof(buffer->data));
	}
	
	emu_wait_buffer(buffer);
}

// Main emulator processing thread
static void * emu_thread(void * args)
{
	uint64_t time = get_timestamp();
	// Timestamp to calc timeouts
	uint64_t emu_timestamp = time;

	puts("Emulator is started");
	nyamodbus_slave_init(emu_device);
	while(emu_running)
	{
		time = get_timestamp();
		
		nyamodbus_slave_tick(emu_device, time - emu_timestamp);
		nyamodbus_slave_main(emu_device);
		
		emu_timestamp = time;
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
		
		buffer_reset(&master_slave);
		buffer_reset(&slave_master);
		
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

// Wait tx to free
static void remu_wait(void)
{
	emu_wait_buffer(&master_slave);
}

// Send data to emulator
//   data: data to send
//   size: size of data
// return: true, if ok
void emu_send(const uint8_t * data, uint8_t size)
{
	emu_send_buffer(&master_slave, data, size);
	nyamodbus_slave_timeout(emu_device);
}

// Dump buffer
//   name: Name of buffer
//   data: data to send
//   size: size of data
void emu_dump_buffer(const char * name, const uint8_t * data, uint8_t size)
{
	int i;
	printf("%s: ", name);
	for(i = 0; i < size; i++)
	{
		printf("%02x ", data[i]);
		
		if((i % 0x10) == 0x0f) puts("");
	}
	puts("");
}

// Send slave emulator modbus data
//   data: data to send
//   size: size of data
// return: true, if ok
bool emu_slave_send(const uint8_t * data, uint8_t size)
{
	emu_dump_buffer("slave send", data, size);
	emu_send_buffer(&slave_master, data, size);
	
	return true;
}

// Receive slave emulator modbus data
//   data: data to read
//   size: size of buffer, size of readed data if result is true
// return: true, if ok
bool emu_slave_receive(uint8_t * data, uint8_t * size)
{
	emu_receive_buffer(&master_slave, data, size);
	
	if(*size > 0)
	    emu_dump_buffer("slave received", data, *size);
	
	return (*size > 0);
}

// Send slave emulator modbus data
//   data: data to send
//   size: size of data
// return: true, if ok
bool emu_master_send(const uint8_t * data, uint8_t size)
{
	emu_dump_buffer("master send", data, size);
	emu_send_buffer(&master_slave, data, size);
	
	return true;
}

// Receive slave emulator modbus data
//   data: data to read
//   size: size of buffer, size of readed data if result is true
// return: true, if ok
bool emu_master_receive(uint8_t * data, uint8_t * size)
{
	emu_receive_buffer(&slave_master, data, size);
	
	if(*size > 0)
		emu_dump_buffer("master received", data, *size);
	
	return (*size > 0);
}
