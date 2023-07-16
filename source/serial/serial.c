//
// Nyamodbus library v1.0.0 serial
//

#include "serial.h"
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <nyamodbus/nyamodbus_utils.h>

static str_nyamodbus_state        state;
static str_nyamodbus_master_state master_state;

bool serial_master_send(const uint8_t * data, uint8_t size);
bool serial_master_receive(uint8_t * data, uint8_t * size);

static const str_modbus_io        io = {
	.send           = serial_master_send,
	.receive        = serial_master_receive
};

// Modbus slave state
const str_nyamodbus_device modbus_master = {
	.io =    &io,
	.state = &state
};

// Is serial runnung
static bool                                serial_running = false;

// File descriptor
static int                                 serial_fd = -1;

// Emulator device
static const str_nyamodbus_master_device * serial_device = 0;

// Thread control
static pthread_t                           serial_thread_id;

// Get current timestamp
uint64_t get_timestamp(void)
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
	
	return tv.tv_sec*1000000ULL + tv.tv_usec;
}

int get_unreaded_bytes(void)
{
	int bytes_available;
	ioctl(serial_fd, FIONREAD, &bytes_available);
	return bytes_available;
}

// Send slave emulator modbus data
//   data: data to send
//   size: size of data
// return: true, if ok
bool serial_master_send(const uint8_t * data, uint8_t size)
{
	int writed = write(serial_fd, data, size);
	
	return (writed == size);
}

// Receive slave emulator modbus data
//   data: data to read
//   size: size of buffer, size of readed data if result is true
// return: true, if ok
bool serial_master_receive(uint8_t * data, uint8_t * size)
{
	int req = get_unreaded_bytes();
	if(req > 0)
	{
		if(req > *size) { req = *size; }
		
		int readed = read(serial_fd, data, req);
		
		*size = readed;
		
		return readed > 0;
	}
	else
		return false;
}

// Main emulator processing thread
static void * serial_thread(void * args)
{
	uint64_t time = get_timestamp();
	// Timestamp to calc timeouts
	uint64_t serial_timestamp = time;

	puts("Serial is started");
	nyamodbus_master_init(serial_device);
	while(serial_running)
	{
		time = get_timestamp();
		
		nyamodbus_master_tick(serial_device, time - serial_timestamp);
		nyamodbus_master_main(serial_device);
		
		serial_timestamp = time;
		usleep(1000);
	}
	
	puts("Serial is stopped");
	return 0;
}

// Open device
//    dev: path to tty device
// return: true, if opened
bool mbserial_open(const char * dev)
{
	int fd = open(dev, O_RDWR | O_NOCTTY | O_SYNC);  
	
	if(!fd) 
		return false;
	

    // Set appropriate serial and line-discipline options
    struct termios o;

    if (tcgetattr(fd, &o) != 0) {
		puts("tcgetattr error");
		return -1;
    }

    /* 8 bits, no parity, 1 stop bit, no hardware flow control */
    o.c_cflag &= ~(PARENB | CSTOPB | CSIZE | CRTSCTS);
    o.c_cflag |= CS8;

    /* enable receiver, ignore status lines */
    o.c_cflag |= CREAD | CLOCAL;

    /* no software flow control */
    o.c_iflag &= ~(IXON | IXOFF | IXANY);

    /* Operate in raw mode without echo or signaling */
    o.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    /* disable output processing */
    o.c_oflag &= ~OPOST;

    /* Perform pure timed reads */
    o.c_cc[VMIN] = 0;
    o.c_cc[VTIME] = 1;

    if (cfsetispeed(&o, B9600) != 0) {
		puts("cfsetispeed error");
		close(fd);
		return false;
    }

    /* setup inpit/output speed */
    if (cfsetospeed(&o, B9600) != 0) {
		puts("cfsetospeed error");
		close(fd);
		return false;
    }

    /* set attirbutes */
    cfmakeraw(&o);
	
	/* apply the settings, and flush any input/output data */
    if ((tcsetattr(fd, TCSANOW, &o) != 0) || (tcflush(fd, TCIFLUSH) != 0)) {
		puts("tcsetattr or tcflush error ");
		close(fd);
		return false;
    }
	
	serial_fd = fd;
    return true;
}

// Start serial modbus master on tty service
//    dev: path to tty device
// device: device config
// return: true, if started
bool mbserial_start(const char * dev, const str_nyamodbus_master_device * device)
{
	if(!serial_running)
	{
		if(mbserial_open(dev))
		{
			pthread_attr_t attr;
			
			serial_device = device;
			serial_running = true;
			
			pthread_attr_init(&attr);
			pthread_create(&serial_thread_id, &attr, serial_thread, 0);
			
			return true;
		}
		else
		{
			puts("Cannot open tty device");
			return false;
		}
	}
	else
	{
		puts("Serial is already running");
		return false;
	}
}

// Stop serial modbus device
void mbserial_stop(void)
{
	if(serial_running)
	{
		serial_running = false;
		pthread_join(serial_thread_id, 0);
		
		close(serial_fd);
		serial_fd = -1;
	}
}
