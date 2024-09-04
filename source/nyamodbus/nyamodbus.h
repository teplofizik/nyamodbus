//
// Nyamodbus library v1.1.0
//

#include <stdint.h>
#include <stdbool.h>

#ifndef _NYAMODBUS_H
#define _NYAMODBUS_H

#include "nyamodbus_config.h"

#ifdef __cplusplus
extern "C" {
#endif

	// Parse step
	typedef enum {
		STEP_WAIT_SLAVE,
		STEP_WAIT_CODE,
		STEP_WAIT_ADDRESS,
		STEP_WAIT_COUNT,
		STEP_WAIT_SIZE,
		STEP_WAIT_DATA,
		STEP_WAIT_CUSTOM,
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
		ERROR_PARITY          = 8, // The slave detected a parity error when reading the extended memory. The master can repeat the request, but usually in such cases, repairs are required.
		
		ERROR_TIMEOUT         = 100, // Timeout error
	} enum_nyamodbus_error;

	// Is device still sending data
	typedef bool (*nyamb_getstatus)(void);
	
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

	// Read digital status
	//     id: index of contact
	// status: where to write status 
	// return: error code
	typedef enum_nyamodbus_error (*nyamb_readdigital)(uint16_t id, bool * status);

	// Read analog inputs
	//     id: index of input
	// result: where to write result 
	// return: error code
	typedef enum_nyamodbus_error (*nyamb_readanalog)(uint16_t id, uint16_t * result);

	// Write coil
	//     id: index of coil
	// status: coil status 
	// return: error code
	typedef enum_nyamodbus_error (*nyamb_writecoil)(uint16_t id, bool status);

	// Write holding register
	//     id: index of register
	// value: value 
	// return: error code
	typedef enum_nyamodbus_error (*nyamb_writeholding)(uint16_t id, uint16_t value);

	// Read device information
	//  object: object id
	//  return: pointer to id string or 0
	typedef const char * (*nyamb_readdeviceinfo)(uint8_t object);

	// Prototype of function to parse modbus packet
	// context: device context
	//    data: data
	//    size: size of data
	typedef void (*nyamb_on_valid_packet)(void * context, const uint8_t * data, uint16_t size);
	
	// Prototype of function to driver event
	// context: device context
	typedef void (*nyamb_driver_event)(void * context);
	
	// Buffer
	typedef struct {
		// Data buffer
		uint8_t  data[NYAMODBUS_BUFFER_SIZE];

		uint16_t size;      // buffer size
		uint16_t expected;  // wait index
		uint16_t added;     // added index
	} str_nyamodbus_buffer;
	
	typedef struct {
		// Send function
		nyamb_send           send;
		
		// Receive function
		nyamb_receive        receive;
		
		// Is sending
		nyamb_getstatus      is_txbusy;
	} str_modbus_io;
	
	// Driver state
	typedef struct {
        // Parse valid packet
		nyamb_on_valid_packet   on_valid_packet;
		
		// Received invalid packet
        nyamb_driver_event      on_invalid_packet;
		
		// Received invalid packet
        nyamb_driver_event      on_timeout;
		
		// Any data are received
		nyamb_driver_event      on_data;
    } str_nyamodbus_driver;
    
	// Driver state
	typedef struct {
		// Parse step
		enum_nyamodbus_parse_step step;
		
		// Packet expects data section
		bool                      has_data;
		
		// Custom header format
		bool                      custom_header;
		
		// custom request size
		uint8_t                   custom_header_size;
		
		// Time after last data
		uint32_t                  elapsed_us;
		
		// Is master busy
		bool                      busy;
		
		// rx buffer
		str_nyamodbus_buffer      buffer;
	} str_nyamodbus_state;
	
	// Modbus driver config and state
	typedef struct {
		// Modbus IO functions
		const str_modbus_io        * io;
		// Modbus buffers for packet receiving
		str_nyamodbus_state        * state;
	}
	str_nyamodbus_device;

	// Init modbus state
	// device: device context
	void nyamodbus_init(const str_nyamodbus_device * device);

	// Main processing cycle
	//  driver: functions to process packets
	// context: driver context
	// device: device context
	void nyamodbus_main(const str_nyamodbus_device * device, const str_nyamodbus_driver * driver, void * context);
	
	// Send packet
	// device: device context
	//   data: data without crc
	//   size: data size
	void nyamodbus_send_packet(const str_nyamodbus_device * device, const uint8_t * data, uint8_t size);

	// Trigger modbus timeout (parse received data)
	//  device: device context
	//  driver: functions to process packets
	// context: driver context
	void nyamodbus_timeout(const str_nyamodbus_device * device, const str_nyamodbus_driver * driver, void * context);

	// Reset modbus state
	void nyamodbus_reset(const str_nyamodbus_device * device);

	// Start timer
	// device: device context
	void nyamodbus_start_timeout(const str_nyamodbus_device * device);

	// Reset modbus timeout (while receiving data)
	// device: device context
	void nyamodbus_reset_timeout(const str_nyamodbus_device * device);

	// Is busy
	// device: device context
	bool nyamodbus_is_busy(const str_nyamodbus_device * device);

	// Tick modbus timer
	//  device: device context
	//  driver: functions to process packets
	// context: driver context
	//   usecs: useconds after last call
	void nyamodbus_tick(const str_nyamodbus_device * device, const str_nyamodbus_driver * driver, void * context, uint32_t usecs);

#ifdef __cplusplus
};
#endif

#endif // _NYAMODBUS_H
