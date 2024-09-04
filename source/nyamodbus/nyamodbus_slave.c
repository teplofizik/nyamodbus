//
// Nyamodbus library v1.0.0 slave
//

#include "nyamodbus_slave.h"
#include "nyamodbus_utils.h"
#include <string.h>
#include <stdio.h>

static void nyamodbus_slave_on_valid_packet(void * context, const uint8_t * data, uint16_t size);
static void nyamodbus_slave_on_invalid_packet(void * context);
static void nyamodbus_slave_on_data(void * context);

const str_nyamodbus_driver slave_driver = {
	.on_data            = nyamodbus_slave_on_data,
	.on_valid_packet    = nyamodbus_slave_on_valid_packet,
	.on_invalid_packet  = nyamodbus_slave_on_invalid_packet,
	.on_timeout         = 0
};

// Send error packet
//   device: device context
// function: function code
//    error: error code
static void nyamodbus_slave_send_error(const str_nyamodbus_slave_device * device, uint8_t function, enum_nyamodbus_error error)
{
	uint8_t buffer[10];
	
	buffer[0] = *device->address;
	buffer[1] = function | 0x80;
	buffer[2] = (uint8_t)error;
	
	nyamodbus_send_packet(device->device, buffer, 3);
}

// Read digital values
//   device: device context
// function: function code
//  address: start address
//    count: register count
//  readfunc: function to read data
static enum_nyamodbus_error nyamodbus_slave_readdigital(const str_nyamodbus_slave_device * device, uint8_t function, uint16_t address, uint16_t count, nyamb_readdigital readfunc)
{
	enum_nyamodbus_error error = ERROR_NO_FUNCTION;
	uint8_t result[NYAMODBUS_OUTPUT_BUFFER_SIZE];
	uint16_t i;
	uint8_t bytes = (count + 7) / 8;
	uint8_t value = 0;
	
	result[0] = *device->address; // slave address
	result[1] = function;                 // function code
	result[2] = bytes;                    // bytes after header
	
	bytes = 3;
	for(i = 0; i < count; i++)
	{
		uint8_t bit = i & 0x7;
		uint16_t reg = address + i;
		
		bool contact = false;
		error = readfunc(reg, &contact);
		
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 1)
		printf("    REG: %04x = %d\n", reg, contact ? 1 : 0);
#endif
		if(error != ERROR_OK)
			break;
		
		if(contact)
			value |= (1 << bit);
		
		if((i & 0x07) == 0x07)
		{
			result[bytes++] = value;
			value = 0;
		}
	}
	
	if((i & 0x07) != 0x00)
	{
		result[bytes++] = value;
	}
	
	if(error == ERROR_OK)
		nyamodbus_send_packet(device->device, result, bytes);
	
	return error;
}

// Read analog values
//   device: device context
// function: function code
//  address: start address
//    count: register count
//  readfunc: function to read data
static enum_nyamodbus_error nyamodbus_slave_readanalog(const str_nyamodbus_slave_device * device, uint8_t function, uint16_t address, uint16_t count, nyamb_readanalog readfunc)
{
	enum_nyamodbus_error error = ERROR_NO_FUNCTION;
	uint8_t result[NYAMODBUS_OUTPUT_BUFFER_SIZE];
	uint16_t i;
	uint8_t bytes = count * 2;

	if((count == 0) || (bytes + 5 < NYAMODBUS_OUTPUT_BUFFER_SIZE))
	{
		result[0] = *device->address; // slave address
		result[1] = function;                 // function code
		result[2] = bytes;                    // bytes after header
		
		bytes = 3;
		for(i = 0; i < count; i++)
		{
			uint16_t reg = address + i;
			
			uint16_t value = 0;
			error = readfunc(reg, &value);
			
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 1)
			printf("    REG: %04x = %d\n", reg, value);
#endif
			if(error != ERROR_OK)
				break;
			
			result[bytes++] = (value >> 8) & 0xFF;
			result[bytes++] = value & 0xFF;
		}
		
		if(error == ERROR_OK)
			nyamodbus_send_packet(device->device, result, bytes);
		
		return error;
	}
	else
		return ERROR_INV_REQ_VALUE;
}

// Read device information
//      id: object id
//   value: buffer
//    size: buffer size / result size 	
//  return: error code
static enum_nyamodbus_error nyamodbus_slave_readdeviceinfo(const char * id, uint8_t * value, uint8_t * size)
{
	if(id != 0)
	{
		uint8_t tocopy = strlen(id);
		if(tocopy > *size)
			tocopy = *size;
		
		strncpy((char*)value, id, tocopy);
		*size = tocopy;
		return ERROR_OK;
	}
	else
		return ERROR_NO_DATAADDRESS;
}

// Process packet
// device: device context
//   data: packet data
//   size: packet size include crc
// return: true, if correct
static enum_nyamodbus_error nyamodbus_slave_process(const str_nyamodbus_slave_device * device, const uint8_t * data, uint8_t size, bool broadcast)
{
	enum_nyamodbus_error error = ERROR_NO_FUNCTION;
	enum_modbus_function_code func = (enum_modbus_function_code)data[1];
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
	printf("   FUNC: %d\n", func);
#endif
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
			if(device->readcoils)
			{
				error = nyamodbus_slave_readdigital(device, FUNCTION_READ_COIL, address, count, device->readcoils);
			}
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 0)
			else
				puts("    No handler: device->readcoils");
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
			if(device->readcontacts)
			{
				error = nyamodbus_slave_readdigital(device, FUNCTION_READ_CONTACTS, address, count, device->readcontacts);
			}
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 0)
			else
				puts("    No handler: device->readcontacts");
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
			if(device->readholding)
			{
				error = nyamodbus_slave_readanalog(device, FUNCTION_READ_HOLDING, address, count, device->readholding);
			}
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 0)
			else
				puts("    No handler: device->readholding");
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
			if(device->readanalog)
			{
				error = nyamodbus_slave_readanalog(device, FUNCTION_READ_INPUTS, address, count, device->readanalog);
			}
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 0)
			else
				puts("    No handler: device->readanalog");
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

			if(device->writecoil)
			{
				error = device->writecoil(address, (value & 0x01) != 0);
				
				if(error == ERROR_OK)
					nyamodbus_send_packet(device->device, data, 6);
			}
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 0)
			else
				puts("    No handler: device->writecoil");
#endif
		}
		break;
		
	case FUNCTION_WRITE_HOLDING_SINGLE:
		// AH AL VH VL
		{
			uint16_t address = get_u16_value(data, 2);
			uint16_t value   = get_u16_value(data, 4);
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 1)
			printf("   WRITE_HOLDING: REG %04x = %04x\n", address, value);
#endif
			if(device->writeholding)
			{
				error = device->writeholding(address, value);
				
				if(error == ERROR_OK)
					nyamodbus_send_packet(device->device, data, 6);
			}
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 0)
			else
				puts("    No handler: device->writeholding");
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
			uint16_t count   = get_u16_value(data, 4);
			uint8_t bytes    = data[6];
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 1)
			printf("   WRITE_COIL_MULTI: %04x count %04x (%d bytes data)\n", address, count, bytes);
#endif
			if(bytes * 8 >= count)
			{
				if(device->writecoil)
				{
					uint16_t i = 0;
					for(i = 0; i < count; i++)
					{
						uint8_t  offset = 7 + (i / 16) * 2;
						uint8_t  bit  = (i % 0x10);
						uint16_t mask = get_u16_value(data, offset);
						uint16_t reg  = address + i;
						bool value = (mask & (1 << bit)) != 0;
						
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 0)
						printf("    REG %04x = %d\n", reg, value ? 1 : 0);
#endif
						error = device->writecoil(reg, value);
						if(error != ERROR_OK)
							break;
					}
					if((error == ERROR_OK) && (data[0] != 255))
					{
						uint8_t result[6];
						result[0] = data[0]; // slave address
						result[1] = func;    // function code
						set_u16_value(result, 2, address);
						set_u16_value(result, 4, count);
						
						nyamodbus_send_packet(device->device, result, 6);
					}
				}
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 0)
				else
					puts("    No handler: device->writecoil");
#endif
			}
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 0)
			else
			{
				puts("    Invalid count/size");
			}
#endif
		}
		break;
		
	case FUNCTION_WRITE_HOLDING_MULTI:
		// AH AL CH CL SZ DATA
		{
			uint16_t address = get_u16_value(data, 2);
			uint16_t count   = get_u16_value(data, 4);
			uint8_t bytes    = data[6];
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 1)
			printf("   WRITE_HOLDING_MULTI: %04x count %04x  (%d bytes data)\n", address, count, bytes);
#endif
			if(bytes == count * 2)
			{
				if(device->writeholding)
				{
					uint16_t i = 0;
					for(i = 0; i < count; i++)
					{
						uint16_t value = get_u16_value(data, 7 + i * 2);
						uint16_t reg   = address + i;
						
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 1)
						printf("    REG %04x = %04x\n", reg, value);
#endif
						error = device->writeholding(reg, value);
						if(error != ERROR_OK)
							break;
					}
					if((error == ERROR_OK) && (data[0] != 255))
					{
						uint8_t result[6];
						result[0] = data[0]; // slave address
						result[1] = func;    // function code
						set_u16_value(result, 2, address);
						set_u16_value(result, 4, count);
						
						nyamodbus_send_packet(device->device, result, 6);
					}
				}
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 0)
				else
					puts("    No handler: device->writeholding");
#endif
			}
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 0)
			else
			{
				puts("    Invalid count/size");
			}
#endif
		}
		break;
		
	case FUNCTION_REPORT_SLAVE_ID:
		break;
		
	case FUNCTION_READ_DEVICE_IDENTIFICATION:
		{
			uint8_t subfunc    = data[2];
			uint8_t product_id = data[3];
			//uint8_t object     = data[4];
			
			if(subfunc == 0x0E)
			{
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 1)
				printf("   READ_DEVICE_IDENTIFICATION: product %02x\n", product_id);
#endif
				if(device->readdeviceinfo)
				{
					uint8_t obj;
					uint8_t bytes = 8;
					uint8_t result[NYAMODBUS_OUTPUT_BUFFER_SIZE];
					result[0] = *device->address; // slave address
					result[1] = data[1];                 // function code
					result[2] = subfunc;
					result[3] = product_id;
					result[4] = 1; // conformity
					result[5] = 0; // more follows
					result[6] = 0; // next object id
					result[7] = 3;
					
					for(obj = 0; obj < 3; obj++)
					{
						uint8_t   buffer_size = NYAMODBUS_OUTPUT_BUFFER_SIZE - bytes - 2;
						uint8_t * dst = &result[bytes + 2];
						const char * id = device->readdeviceinfo(obj);
						
						error = nyamodbus_slave_readdeviceinfo(id, dst, &buffer_size);
						if(error != ERROR_OK)
							break;
						
						result[bytes++] = obj;
						result[bytes++] = buffer_size;
						
						bytes += buffer_size;
					}
					
					nyamodbus_send_packet(device->device, result, bytes);
				}
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 0)
				else
					puts("    No handler: device->config->readdeviceinfo");
#endif
			}
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 0)
			else
				puts("Unkonown subfunction code for 43 function");
#endif
		}
		break;
	}
	
	return error;
}

// Any data received
static void nyamodbus_slave_on_data(void * context)
{
	str_nyamodbus_slave_device * device = (str_nyamodbus_slave_device *)context;
	
	nyamodbus_start_timeout(device->device);
	nyamodbus_reset_timeout(device->device);
}

// Function to parse modbus packet
//   data: data
//   size: size of data
static void nyamodbus_slave_on_valid_packet(void * context, const uint8_t * data, uint16_t size)
{
	str_nyamodbus_slave_device * device = (str_nyamodbus_slave_device *)context;
	uint8_t slave     = data[0];
	bool    broadcast = (slave == 255);
	
	// Check slave address
	if((slave == *device->address) || broadcast)
	{
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
		printf("  Slave ok: %02x\n", slave);
#endif
		enum_nyamodbus_error error = nyamodbus_slave_process(device, data, size, broadcast);
		if((error != ERROR_OK) && !broadcast)
		{
			// Send error packet
			nyamodbus_slave_send_error(device, data[1], error);
		}
	}
	else
	{
#if defined(DEBUG_OUTPUT) && (DEBUG_OUTPUT > 2)
		printf("  Slave invalid: %02x\n", slave);
#endif
	}
}

// Invalid packet
static void nyamodbus_slave_on_invalid_packet(void * context)
{
	
}

// Init slave modbus state
// device: device context
void nyamodbus_slave_init(const str_nyamodbus_slave_device * device)
{
	nyamodbus_init(device->device);
}

// Init slave modbus state
// device: device context
void nyamodbus_slave_main(const str_nyamodbus_slave_device * device)
{
	nyamodbus_main(device->device, &slave_driver, (void*)device);
}

// Trigger modbus timeout (parse received data)
void nyamodbus_slave_timeout(const str_nyamodbus_slave_device * device)
{
	nyamodbus_timeout(device->device, &slave_driver, (void *)device);
}

// Trigger modbus timeout (parse received data)
//  device: device context
//   usecs: useconds after last call
// context: driver context
void nyamodbus_slave_tick(const str_nyamodbus_slave_device * device, uint32_t usecs)
{
	nyamodbus_tick(device->device, &slave_driver, (void*)device, usecs);
}

// Reset modbus state
void nyamodbus_slave_reset(const str_nyamodbus_slave_device * device)
{
	nyamodbus_reset(device->device);
}
