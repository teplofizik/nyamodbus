//
// Nyamodbus library v1.0.0
//

#include "nyamodbus.h"
#include <string.h>
#include <stdio.h>

// Init modbus state
// device: device context
void nyamodbus_init(str_nyamodbus_device * device)
{
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
	puts("nyamodbus_init");
	
	if(!device->state)
		puts("Empty pointer to state!");
	
	if(!device->config)
		puts("Empty pointer to config!");
	else
	{
		if(!device->config->send) puts("Empty pointer to config->send!");
		if(!device->config->receive) puts("Empty pointer to config->receive!");
	}
	
#endif

	nyamodbus_reset(device);
}

// Reset modbus state
void nyamodbus_reset(str_nyamodbus_device * device)
{
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
	puts("nyamodbus_reset");
#endif

	// Init buffer...
	memset(device->state, 0, sizeof(str_nyamodbus_state));
	
	device->state->has_data = false;
	device->state->buffer.size = NYAMODBUS_BUFFER_SIZE;
}

// Detect step by function code
// function: code
//   return: step
static void nyamodbus_decide_steps(str_nyamodbus_device * device, enum_modbus_function_code function)
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

// Swap bytes in u16 value
static uint16_t swap16(uint16_t val) {
  return ((val & 0xFF) << 8) | ((val >> 8) & 0xFF);
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

	return crc;
}

// Get u16 value from packet
//   data: packet data
// offset: offset
// return: value
static uint16_t get_u16_value(const uint8_t * data, uint8_t offset)
{
	return (((uint16_t)data[offset]) << 8) | data[offset + 1];
}

// Check packet crc
//   data: packet data
//   size: packet size include crc
// return: true, if correct
static bool nyamodbus_checkcrc(const uint8_t * data, uint8_t size)
{
	uint16_t calc_crc = swap16(nyamodbus_crc(data, size - 2));
	uint16_t exists_crc = get_u16_value(data, size - 2);
	
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 1)
	printf(" Compare crc16: calc %04x and expected %04x\n", calc_crc, exists_crc);
#endif
	return exists_crc == calc_crc;
}

// Process packet
//   data: packet data
//   size: packet size include crc
// return: true, if correct
static enum_nyamodbus_error nyamodbus_process(const uint8_t * data, uint8_t size, bool broadcast)
{
	enum_modbus_function_code func = (enum_modbus_function_code)data[1];
	switch(func)
	{
	case FUNCTION_READ_COIL:
		// AH AL CH CL
		{
			uint16_t address = get_u16_value(data, 2);
			uint16_t count   = get_u16_value(data, 4);
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 1)
			printf("   READ_COIL: %04x-%04x (%d)\n", address, address + count - 1, count);
#endif
		}
		break;
		
	case FUNCTION_READ_CONTACTS:
		// AH AL CH CL
		{
			uint16_t address = get_u16_value(data, 2);
			uint16_t count   = get_u16_value(data, 4);
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 1)
			printf("   READ_CONTACTS: %04x-%04x (%d)\n", address, address + count - 1, count);
#endif
		}
		break;
		
	case FUNCTION_READ_HOLDING:
		// AH AL CH CL
		{
			uint16_t address = get_u16_value(data, 2);
			uint16_t count   = get_u16_value(data, 4);
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 1)
			printf("   READ_HOLDING: %04x-%04x (%d)\n", address, address + count - 1, count);
#endif
		}
		break;
		
	case FUNCTION_READ_INPUTS:
		// AH AL CH CL
		{
			uint16_t address = get_u16_value(data, 2);
			uint16_t count   = get_u16_value(data, 4);
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 1)
			printf("   READ_INPUTS: %04x-%04x (%d)\n", address, address + count - 1, count);
#endif
		}
		break;
		
	case FUNCTION_WRITE_COIL_SINGLE: 
		// AH AL VH VL
		{
			uint16_t address = get_u16_value(data, 2);
			uint16_t value   = get_u16_value(data, 4);
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 1)
			printf("   WRITE_COIL: %04x = %04x\n", address, value);
#endif
		}
		break;
		
	case FUNCTION_WRITE_HOLDING_SINGLE:
		// AH AL VH VL
		{
			uint16_t address = get_u16_value(data, 2);
			uint16_t value   = get_u16_value(data, 4);
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 1)
			printf("   WRITE_HOLDING: %04x = %04x\n", address, value);
#endif
		}
		break;
		
	case FUNCTION_READ_EXCEPTION_STATUS:
	
		break;
		
	case FUNCTION_DIAGNOSTIC:
		break;
		
	case FUNCTION_WRITE_COIL_MULTI:
		// AH AL CH CL SZ DATA
		{
			uint16_t address = get_u16_value(data, 2);
			uint16_t value   = get_u16_value(data, 4);
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 1)
			printf("   WRITE_COIL_MULTI: %04x count %04x\n", address, value);
#endif
		}
		break;
		
	case FUNCTION_WRITE_HOLDING_MULTI:
		// AH AL CH CL SZ DATA
		{
			uint16_t address = get_u16_value(data, 2);
			uint16_t value   = get_u16_value(data, 4);
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 1)
			printf("   WRITE_HOLDING_MULTI: %04x count %04x\n", address, value);
#endif
		}
		break;
		
	case FUNCTION_REPORT_SLAVE_ID:
		break;
		
	case FUNCTION_READ_DEVICE_IDENTIFICATION:
		break;
	}
	
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
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
		puts("======================");
		printf(" Slave = %02x\n", byte);
#endif
		break;
		
	case STEP_WAIT_CODE:
		buffer->expected = 4;
		buffer->data[buffer->added++] = byte;
		device->state->step = STEP_WAIT_ADDRESS;
		nyamodbus_decide_steps(device, (enum_modbus_function_code)byte);
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
		printf(" Code = %02x\n", byte);
#endif
		break;
		
	case STEP_WAIT_ADDRESS:
		buffer->data[buffer->added++] = byte;
		if(buffer->added == buffer->expected)
		{
			buffer->expected = 6;
			device->state->step = STEP_WAIT_COUNT;
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
			printf(" Address = %02x%02x\n", buffer->data[2], buffer->data[3]);
#endif
		}
		break;
		
	case STEP_WAIT_COUNT:
		buffer->data[buffer->added++] = byte;
		if(buffer->added == buffer->expected)
		{
			if(device->state->has_data)
			{
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
				printf(" Value = %02x%02x NEXT:SIZE\n", buffer->data[4], buffer->data[5]);
#endif
				
				buffer->expected = 7;
				device->state->step = STEP_WAIT_SIZE;
			}
			else
			{
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
				printf(" Value = %02x%02x NEXT:CRC\n", buffer->data[4], buffer->data[5]);
#endif

				buffer->expected = 8;
				device->state->step = STEP_WAIT_CRC;
			}
		}
		break;
		
	case STEP_WAIT_SIZE:
		buffer->data[buffer->added++] = byte;
		buffer->expected = 7 + byte;
		device->state->step = STEP_WAIT_DATA;
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
		printf("  Size = %d\n", byte);
#endif
		break;
		
	case STEP_WAIT_DATA:
		buffer->data[buffer->added++] = byte;
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
		printf("  Data = %02x\n", byte);
#endif
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
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
			printf(" Crc16 = %02x%02x\n", buffer->data[buffer->expected - 2], buffer->data[buffer->expected - 1]);
#endif
			// Check crc16...
			if(nyamodbus_checkcrc(buffer->data, buffer->expected))
			{
				uint8_t slave     = buffer->data[0];
				bool    broadcast = (slave == 255);
				
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
				puts("  Crc ok");
#endif
				// Check slave address
				if((slave == *device->config->address) || broadcast)
				{
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
					printf("  Slave ok: %02x\n", slave);
#endif
					enum_nyamodbus_error error = nyamodbus_process(buffer->data, buffer->expected, broadcast);
					if((error != ERROR_OK) && !broadcast)
					{
						// Send error packet
						
					}
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
		if(size > 0)
		{
			uint8_t i;
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 0)
			printf("Readed %d bytes\n", size);
#endif
			for(i = 0; i < size; i++)
			{
				nyamodbus_processbyte(device, buffer[i]);
			}
		}
	}
}
