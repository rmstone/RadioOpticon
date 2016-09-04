#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "SerialPort.h"

int writeSerialPort( int fd, uint8_t * data, int count )
{
	return write( fd, data, count );
}

int readSerialPort( int fd, uint8_t * data, int count )
{
	return read( fd, data, count );
}

int readSerialPortLine( int fd, std::string &str, int count )
{
	int bytesRead = 0;
	char input = 0xFF;
	while( input != '\n' && bytesRead < count )
	{
		read( fd, &input, 1 );
		str += input;
		//printf( "Read %c", input );
		bytesRead++;
	}
	return bytesRead;
}

int openSerialPort( std::string portname )
{
	int fd = open( portname.c_str(), O_RDWR | O_NOCTTY | O_SYNC );
	if( fd < 0 )
	{
		fprintf( stderr, "error %d opening %s: %s", errno, portname.c_str(), strerror( errno ) );
		return 0;
	}
	return fd;
}

void closeSerialPort( int fd )
{
	if( fd != 0 )
		close( fd );
}

int set_interface_attribs( int fd, int speed, int parity )
{
	struct termios tty;
	memset( &tty, 0, sizeof tty );
	if( tcgetattr( fd, &tty ) != 0 )
	{
		fprintf( stderr, "error %d from tcgetattr", errno );
		return -1;
	}

	cfsetospeed( &tty, speed );
	cfsetispeed( &tty, speed );

	tty.c_cflag = ( tty.c_cflag & ~CSIZE ) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;         // disable break processing
	tty.c_lflag = 0;                // no signaling chars, no echo,
	// no canonical processing
	tty.c_oflag = 0;                // no remapping, no delays
	tty.c_cc[VMIN] = 0;            // read doesn't block
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	tty.c_iflag &= ~( IXON | IXOFF | IXANY ); // shut off xon/xoff ctrl

	tty.c_cflag |= ( CLOCAL | CREAD );// ignore modem controls,
	// enable reading
	tty.c_cflag &= ~( PARENB | PARODD );      // shut off parity
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if( tcsetattr( fd, TCSANOW, &tty ) != 0 )
	{
		fprintf( stderr, "error %d from tcsetattr", errno );
		return -1;
	}
	return 0;
}

void set_blocking( int fd, int should_block )
{
	struct termios tty;
	memset( &tty, 0, sizeof tty );
	if( tcgetattr( fd, &tty ) != 0 )
	{
		fprintf( stderr, "error %d from tggetattr", errno );
		return;
	}

	tty.c_cc[VMIN] = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	if( tcsetattr( fd, TCSANOW, &tty ) != 0 )
		fprintf( stderr, "error %d setting term attributes", errno );
}