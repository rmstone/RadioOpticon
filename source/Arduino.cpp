#include "SerialPort.h"
#include "Arduino.h"
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>

#define TEMP_OFFSET 0
#define DIST_THRESHOLD 75
#define FOUND_COUNT_TIMEOUT 400

std::vector<std::string> &split( const std::string &s, char delim, std::vector<std::string> &elems )
{
	std::stringstream ss( s );
	std::string item;
	while( std::getline( ss, item, delim ) )
	{
		elems.push_back( item );
	}
	return elems;
}


std::vector<std::string> split( const std::string &s, char delim )
{
	std::vector<std::string> elems;
	split( s, delim, elems );
	return elems;
}

//setting thresholds for detection
// 'f' for found count threshold
// 'd' for distance in cm
// 't' for temp offset in tenths of degrees ( 1 deg = 10 value )
Arduino::Arduino( std::string portname )
{
	fd = openSerialPort( portname );
	set_interface_attribs( fd, B57600, 0 );
	set_blocking( fd, 1 );
	uint8_t data[7] = { 't', TEMP_OFFSET, 'd', DIST_THRESHOLD, 'f', ( uint8_t )( FOUND_COUNT_TIMEOUT & 0xFF ), ( uint8_t )( ( FOUND_COUNT_TIMEOUT & 0xFF00 ) >> 8 ) };
	writeSerialPort( fd, (uint8_t*)&data, 7 );
}

Arduino::~Arduino()
{
	closeSerialPort( fd );
}

bool Arduino::isValid()
{
	return fd != 0;
}

void Arduino::ClearFound()
{
	char clear = 'c';
	writeSerialPort( fd, ( uint8_t * )&clear, 1 );
}

SensorState Arduino::CheckSensors()
{
	//SensorState ret = clear;
	//std::string send = "sensor\n";
	//writeSerialPort( fd, ( uint8_t* )send.c_str(), 7 );
	char request = 's';
	writeSerialPort( fd, ( uint8_t* )&request, 1 );
	char found;
	readSerialPort( fd, ( uint8_t* )&found, 1 );
	return found ? personFound : clear;
	//std::string data = "";
	//readSerialPortLine( fd, data, 512 );
	//std::vector<std::string> splitData = split( data, ',' );
	//if( splitData.size() == 19 )
	//{
	//	int ambient = atoi( splitData.at( 0 ).c_str() );
	//	int distance = atoi( splitData.at( 17 ).c_str() );
	//	int angle = atoi( splitData.at( 18 ).c_str() );
	//	for( int i = 1; i < 17; i++ )
	//	{
	//		int value = atoi( splitData.at( i ).c_str() );
	//		if( value > ambient && distance < 50 )
	//		{
	//			ret = personFound;
	//			printf( "Ambient: %d Person: %d Distance %dcm\n", ambient, value, distance );
	//		}
	//	}
	//}
	//return ret;
}