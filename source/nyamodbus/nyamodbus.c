//
// Nyamodbus library v1.0.0
//

#include "nyamodbus.h"
#include "nyamodbus_utils.h"
#include <string.h>
#include <stdio.h>

// Init modbus state
// device: device context
void nyamodbus_init(const str_nyamodbus_device * device)
{
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
	puts("nyamodbus_init");
	
	if(!device->state)
		puts("Empty pointer to state!");
	
	if(!device->io)
		puts("Empty pointer to io!");
	else
	{
		if(!device->io->send) puts("Empty pointer to io->send!");
		if(!device->io->receive) puts("Empty pointer to io->receive!");
	}
	
#endif

	nyamodbus_reset(device);
}

// Reset modbus state
void nyamodbus_reset(const str_nyamodbus_device * device)
{
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
	puts("nyamodbus_reset");
#endif

	// Init buffer...
	memset(device->state, 0, sizeof(str_nyamodbus_state));
	
	device->state->has_data = false;
	device->state->buffer.size = NYAMODBUS_BUFFER_SIZE;
}

// Calc crc16
//   data: packet data
//   size: packet size without crc
// return: crc16
static uint16_t nyamodbus_crc(const uint8_t *data, uint8_t size)
{
	uint16_t crc = 0xFFFF;
	uint8_t i = 0;
	uint8_t bit = 0;

	for(i = 0; i < size; i++)
	{
		crc ^= data[i];

		for(bit = 0; bit < 8; bit++)
		{
			if( crc & 0x0001 )
			{
				crc >>= 1;
				crc ^= 0xA001;
			}
			else
			{
				crc >>= 1;
			}
		}
	}

	return swap_u16(crc);
}

// Send packet
// data: data without crc
// size: data size
void nyamodbus_send_packet(const str_nyamodbus_device * device, const uint8_t * data, uint8_t size)
{
	uint16_t crc = nyamodbus_crc(data, size);
	uint8_t result[NYAMODBUS_OUTPUT_BUFFER_SIZE];
	
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
	printf(" Send packet with size %d\n", size);
#endif

	memcpy(result, data, size);
	set_u16_value(result, size, crc);
	
	device->io->send(result, size + 2);
}

// Check packet crc
//   data: packet data
//   size: packet size include crc
// return: true, if correct
static bool nyamodbus_checkcrc(const uint8_t * data, uint8_t size)
{
	uint16_t calc_crc = nyamodbus_crc(data, size - 2);
	uint16_t exists_crc = get_u16_value(data, size - 2);
	
	bool correct = (calc_crc == exists_crc);
	
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 1)
	printf(" Compare crc16: calc %04x and expected %04x => %s\n", calc_crc, exists_crc, correct ? "ok" : "fail");
#endif
	return correct;
}

// Process byte
// device: device context
//   byte: received byte
static void nyamodbus_processbyte(const str_nyamodbus_device * device, uint8_t byte)
{
	str_nyamodbus_buffer * buffer = &device->state->buffer;
	if(buffer->added < NYAMODBUS_BUFFER_SIZE)
	{
		buffer->data[buffer->added++] = byte;
	}
}

// Is busy
// device: device context
bool nyamodbus_is_busy(const str_nyamodbus_device * device)
{
	return device->state->busy;
}

// Main processing cycle
// device: device context
void nyamodbus_main(const str_nyamodbus_device * device, const str_nyamodbus_driver * driver, void * context)
{
	uint8_t buffer[100];
	uint8_t size = sizeof(buffer);
	
	if(device->io->is_txbusy)
	{
		if(device->io->is_txbusy())
			nyamodbus_reset_timeout(device);
	}
	
	// If something is available to receive...
	if(device->io->receive && device->io->receive(buffer, &size))
	{
		if(size > 0)
		{
			uint8_t i;
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 0)
			printf("Readed %d bytes\n", size);
#endif

			if(driver->on_data)
				driver->on_data(context);

			for(i = 0; i < size; i++)
			{
				nyamodbus_processbyte(device, buffer[i]);
			}
		}
	}
}

// Trigger modbus timeout (parse received data)
//  device: device context
//  driver: functions to process packets
// context: driver context
void nyamodbus_timeout(const str_nyamodbus_device * device, const str_nyamodbus_driver * driver, void * context)
{
	str_nyamodbus_buffer * buffer = &device->state->buffer;
	
	if(buffer->added > 4)
	{
		uint8_t size = buffer->added;
		
		if(nyamodbus_checkcrc(buffer->data, size))
		{
			if(driver->on_valid_packet)
				driver->on_valid_packet(context, buffer->data, size);
		}
		else
		{
			if(driver->on_invalid_packet)
				driver->on_invalid_packet(context);
		}
	}
	else
	{
		if(driver->on_timeout)
			driver->on_timeout(context);
	}
	
	nyamodbus_reset(device);
}

// Tick modbus timer
//  device: device context
//  driver: functions to process packets
// context: driver context
//   usecs: useconds after last call
void nyamodbus_tick(const str_nyamodbus_device * device, const str_nyamodbus_driver * driver, void * context, uint32_t usecs)
{
	if (device->state->busy)
	{
		device->state->elapsed_us += usecs;
		
		if(device->state->elapsed_us >= NYAMODBUS_PACKET_WAIT_TIMEOUT)
		{
			nyamodbus_timeout(device, driver, context);
			
			device->state->busy = false;
		}
	}
}

// Start timer
// device: device context
void nyamodbus_start_timeout(const str_nyamodbus_device * device)
{
	device->state->busy = true;
	device->state->elapsed_us = 0;
}

// Reset modbus timeout (while receiving data)
// device: device context
void nyamodbus_reset_timeout(const str_nyamodbus_device * device)
{
	if (device->state->busy)
	{
		device->state->elapsed_us = 0;
	}
}
