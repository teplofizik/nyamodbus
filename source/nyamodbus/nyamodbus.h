//
// Nyamodbus library v1.0.0
//

#include <stdint.h>
#include <stdbool.h>

#ifndef _NYAMODBUS_H
#define _NYAMODBUS_H

	// Debug mode (0-3)
	#define DEBUG_OUTPUT            3

	// Receive buffer size
	#define NYAMODBUS_BUFFER_SIZE   24

	// Parse step
	typedef enum {
		STEP_WAIT_SLAVE,
		STEP_WAIT_CODE,
		STEP_WAIT_ADDRESS,
		STEP_WAIT_COUNT,
		STEP_WAIT_SIZE,
		STEP_WAIT_DATA,
		STEP_WAIT_CRC
	} enum_nyamodbus_parse_step;

	// Function codes
	typedef enum {
		FUNCTION_READ_COIL                  = 1,
		FUNCTION_READ_CONTACTS              = 2,
		FUNCTION_READ_HOLDING               = 3,
		FUNCTION_READ_INPUTS                = 4,
		FUNCTION_WRITE_COIL_SINGLE          = 5,
		FUNCTION_WRITE_HOLDING_SINGLE       = 6,
		
		FUNCTION_READ_EXCEPTION_STATUS      = 7,
		FUNCTION_DIAGNOSTIC                 = 8,
		
		FUNCTION_WRITE_COIL_MULTI           = 15,
		FUNCTION_WRITE_HOLDING_MULTI        = 16,
		
		FUNCTION_REPORT_SLAVE_ID            = 17,
		
		FUNCTION_READ_DEVICE_IDENTIFICATION = 43
	} enum_modbus_function_code;


	typedef enum {
		ERROR_OK              = 0, // OK
		ERROR_NO_FUNCTION     = 1, // Function code accepted can not be processed
		ERROR_NO_DATAADDRESS  = 2, // The data address specified in the request is not available.
		ERROR_INV_REQ_VALUE   = 3, // The value contained in the query data field is an invalid value.
		ERROR_UNRECOVERABLE   = 4, // An unrecoverable error occurred while the slave attempted to perform the requested action.
		ERROR_LONG_ACTION     = 5, // The slave has accepted the request and processes it, but it takes a long time. This response prevents the host from generating a timeout error.
		ERROR_BUSY            = 6, // The slave is busy processing the command. The master must repeat the message later when the slave is freed.
		ERROR_NEED_DIAGNOSTIC = 7, // The slave can not execute the program function specified in the request. This code is returned for an unsuccessful program request using functions with numbers 13 or 14. The master must request diagnostic information or error information from the slave.
		ERROR_PARITY          = 8  // The slave detected a parity error when reading the extended memory. The master can repeat the request, but usually in such cases, repairs are required.
	} enum_nyamodbus_error;

	// Prototype of function to send modbus data
	//   data: data to send
	//   size: size of data
	// return: true, if ok
	typedef bool (*nyamb_send)(const uint8_t * data, uint8_t size);

	// Prototype of function to receive modbus data
	//   data: data to read
	//   size: size of buffer, size of readed data if result is true
	// return: true, if ok
	typedef bool (*nyamb_receive)(uint8_t * data, uint8_t * size);

	// Read contact status
	//     id: index of contact
	// status: where to write status 
	// return: error code
	typedef enum_nyamodbus_error (*nyamb_readcontacts)(uint16_t id, bool * status);

	// Read analog inputs
	//     id: index of input
	// result: where to write result 
	// return: error code
	typedef enum_nyamodbus_error (*nyamb_readanalog)(uint16_t id, uint16_t * result);

	// Read coil status
	//     id: index of coils
	// status: where to write status 
	// return: error code
	typedef enum_nyamodbus_error (*nyamb_readcoils)(uint16_t id, bool * status);

	// Write coils
	//     id: index of coil
	// status: coil status 
	// return: error code
	typedef enum_nyamodbus_error (*nyamb_writecoils)(uint16_t id, bool status);

	// Read holding register
	//     id: index of register
	//  value: where to write value 
	// return: error code
	typedef enum_nyamodbus_error (*nyamb_readholding)(uint16_t id, uint16_t * value);

	// Write holding register
	//     id: index of register
	// value: value 
	// return: error code
	typedef enum_nyamodbus_error (*nyamb_writeholding)(uint16_t id, uint16_t value);

	// Read device information
	//     id: object id
	//  value: buffer
	//   size: buffer size / result size 	
	// return: error code
	typedef enum_nyamodbus_error (*nyamb_readdeviceinfo)(uint8_t id, uint8_t * value, uint8_t * size);

	// Driver configuration
	typedef struct {
		// Pointer to slave address
		uint8_t *            address;
		
		// Send function
		nyamb_send           send;
		
		// Receive function
		nyamb_receive        receive;
		
		// Read device information
		nyamb_readdeviceinfo readdeviceinfo;
		
		// Read contacts
		nyamb_readcontacts   readcontacts;
		
		// Read analog inputs
		nyamb_readanalog     readanalog;
		
		// Read coil status
		nyamb_readcoils      readcoils;
		
		// Write coil status
		nyamb_writecoils     writecoils;
		
		// Read holding register
		nyamb_readholding    readholding;
		
		// Write holding register
		nyamb_writeholding   writeholding;
	} str_nyamodbus_config;
	
	// Buffer
	typedef struct {
		// Data buffer
		uint8_t  data[NYAMODBUS_BUFFER_SIZE];

		uint16_t size;      // buffer size
		uint16_t expected;  // wait index
		uint16_t added;     // added index
	} str_nyamodbus_buffer;
	
	// Driver state
	typedef struct {
		// Parse step
		enum_nyamodbus_parse_step step;
		
		// Packet expects data section
		bool                      has_data;
		
		// rx buffer
		str_nyamodbus_buffer      buffer;
	} str_nyamodbus_state;
	
	// Modbus driver config and state
	typedef struct {
		const str_nyamodbus_config * config;
		str_nyamodbus_state        * state;
	}
	str_nyamodbus_device;

	// Init modbus state
	// device: device context
	void nyamodbus_init(str_nyamodbus_device * device);

	// Main processing cycle
	// device: device context
	void nyamodbus_main(str_nyamodbus_device * device);
	
	// Reset modbus state
	void nyamodbus_reset(str_nyamodbus_device * device);

#endif // _NYAMODBUS_H
