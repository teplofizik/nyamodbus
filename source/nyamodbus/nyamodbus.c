//
// Nyamodbus library v1.0.0
//

#include "nyamodbus.h"
#include <string.h>

// Init modbus state
// device: device context
void nyamodbus_init(str_nyamodbus_device * device)
{
	nyamodbus_reset(device);
}

// Reset modbus state
void nyamodbus_reset(str_nyamodbus_device * device)
{
	// Init buffer...
	memset(device->state, 0, sizeof(str_nyamodbus_state));
	
	device->state->buffer.size = NYAMODBUS_BUFFER_SIZE;
}

// Detect step by function code
// function: code
//   return: step
static void nyamodbus_decide_steps(str_nyamodbus_device * device, uint8_t function)
{
	device->state->has_data = false;
	
	switch(function)
	{
	case FUNCTION_READ_COIL:
		// AH AL CH CL
		break;
	case FUNCTION_READ_CONTACTS:
		// AH AL CH CL
		break;
	case FUNCTION_READ_HOLDING:
		// AH AL CH CL
		break;
	case FUNCTION_READ_INPUTS:
		// AH AL CH CL
		break;
	case FUNCTION_WRITE_COIL_SINGLE: 
		// AH AL VH VL
		break;
	case FUNCTION_WRITE_HOLDING_SINGLE:
		// AH AL VH VL
		break;
	case FUNCTION_READ_EXCEPTION_STATUS:
		break;
	case FUNCTION_DIAGNOSTIC:
		break;
	case FUNCTION_WRITE_COIL_MULTI:
		// AH AL CH CL SZ DATA
		device->state->has_data = true;
		break;
	case FUNCTION_WRITE_HOLDING_MULTI:
		// AH AL CH CL SZ DATA
		device->state->has_data = true;
		break;
	case FUNCTION_REPORT_SLAVE_ID:
		break;
	case FUNCTION_READ_DEVICE_IDENTIFICATION:
		break;
	}
}

// Calc crc16
static uint16_t nyamodbus_crc(const uint8_t *buf, uint8_t len)
{
	uint16_t crc = 0xFFFF;
	uint8_t i = 0;
	uint8_t bit = 0;

	for(i = 0; i < len; i++)
	{
		crc ^= buf[i];

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

	return crc;
}

// Check packet crc
//   data: packet data
//   size: packet size include crc
// return: true, if correct
static bool nyamodbus_checkcrc(const uint8_t * data, uint8_t size)
{
	uint16_t calc_crc = nyamodbus_crc(data, size - 2);
	uint16_t exists_crc = (((uint16_t)data[size - 2]) << 8) | data[size - 1];
	
	return exists_crc == calc_crc;
}

// Process packet
//   data: packet data
//   size: packet size include crc
// return: true, if correct
static enum_nyamodbus_error nyamodbus_process(const uint8_t * data, uint8_t size)
{
	return ERROR_OK;
}

// Process byte
// device: device context
// byte: received byte
static void nyamodbus_processbyte(str_nyamodbus_device * device, uint8_t byte)
{
	str_nyamodbus_buffer * buffer = &device->state->buffer;
	switch(device->state->step)
	{
	case STEP_WAIT_SLAVE:
		buffer->added = 0;
		buffer->expected = 1;
		buffer->data[buffer->added++] = byte;
		device->state->step = STEP_WAIT_CODE;
		break;
		
	case STEP_WAIT_CODE:
		buffer->expected = 4;
		buffer->data[buffer->added++] = byte;
		device->state->step = STEP_WAIT_ADDRESS;
		nyamodbus_decide_steps(device, byte);
		break;
		
	case STEP_WAIT_ADDRESS:
		buffer->data[buffer->added++] = byte;
		if(buffer->added == buffer->expected)
		{
			buffer->expected = 6;
			device->state->step = STEP_WAIT_COUNT;
		}
		break;
		
	case STEP_WAIT_COUNT:
		buffer->data[buffer->added++] = byte;
		if(buffer->added == buffer->expected)
		{
			if(device->state->has_data)
			{
				buffer->expected = 7;
				device->state->step = STEP_WAIT_SIZE;
			}
			else
			{
				buffer->expected = 8;
				device->state->step = STEP_WAIT_CRC;
			}
		}
		break;
		
	case STEP_WAIT_SIZE:
		buffer->data[buffer->added++] = byte;
		buffer->expected = 7 + byte;
		device->state->step = STEP_WAIT_DATA;
		break;
		
	case STEP_WAIT_DATA:
		buffer->data[buffer->added++] = byte;
		if(buffer->added == buffer->expected)
		{
			buffer->expected += 2;
			device->state->step = STEP_WAIT_CRC;
		}
		break;
		
	case STEP_WAIT_CRC:
		buffer->data[buffer->added++] = byte;
		if(buffer->added == buffer->expected)
		{
			// Check crc16...
			if(nyamodbus_checkcrc(buffer->data, buffer->expected))
			{
				enum_nyamodbus_error error = nyamodbus_process(buffer->data, buffer->expected);
				if(error != ERROR_OK)
				{
					// Send error packet
					
				}
			}
			device->state->step = STEP_WAIT_SLAVE;
		}
		break;
	}
}

// Main processing cycle
// device: device context
void nyamodbus_main(str_nyamodbus_device * device)
{
	uint8_t buffer[100];
	uint8_t size = sizeof(buffer);
	
	// If something is available to receive...
	if(device->config->receive && device->config->receive(buffer, &size))
	{
		uint8_t i;
		for(i = 0; i < size; i++)
		{
			nyamodbus_processbyte(device, buffer[i]);
		}
	}
}
