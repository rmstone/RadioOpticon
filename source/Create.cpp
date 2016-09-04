#include <termios.h>
#include <unistd.h>
#include <cstdlib>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "Create.h"
#include "SerialPort.h"


inline uint64_t GetTimeMs()
{
	struct timeval tp;
	gettimeofday( &tp, NULL );
	return ( tp.tv_sec * 1000 ) + ( tp.tv_usec / 1000 );
}

Create::Create( std::string portname )
{
	fd = openSerialPort( portname );
	set_interface_attribs( fd, B57600, 0 );
	set_blocking( fd, 1 );
	uint8_t data[2];
	data[0] = 128;
	data[1] = 130;
	writeSerialPort( fd, ( uint8_t* )&data, 2 );
	_virtualWall = false;
	_turning = false;
	_chargingState = 0;
	_batteryCharge = 65535;
	_updateSensorTimeout = GetTimeMs();
}

Create::~Create()
{
	Stop();
	closeSerialPort( fd );
}

bool Create::isValid()
{
	return fd != 0;
}

void Create::DriveAround()
{
	uint8_t data[1];
	data[0] = 135;
	writeSerialPort( fd, ( uint8_t* )&data, 1 );
}

void Create::Driving()
{
	UpdateSensors();
	if( _virtualWall )
	{
		if( rand() % 2 )
			TurnLeft();
		else
			TurnRight();
		printf( "Virtual Wall\n" );
	}
	else if( _bumpLeft )
		TurnRight();
	else if( _bumpRight )
		TurnLeft();
	else if( _turning )
	{
		if( _turningTimeout < GetTimeMs() )
		{
			_turning = false;
			DriveForward();
		}
	}
	else
		DriveForward();
}

void Create::Undock()
{
	Backup( 1000000 );
	TurnAround();
}

bool Create::IsBatteryLow()
{
	return false;
	//return _batteryCharge < BATTERY_THRESHOLD;
}

void Create::DriveForward()
{
	uint8_t data[5] = { 137, 0, 100, 0x7F, 0xFF };
	writeSerialPort( fd, ( uint8_t* )&data, 5 );
}

void Create::Stop()
{
	_turning = false;
	uint8_t data[5] = { 137, 0, 0, 0, 0 };
	writeSerialPort( fd, ( uint8_t* )&data, 5 );
}

void Create::Backup( uint64_t uSec )
{
	uint8_t data[5] = { 137, 0xFF, 0x6A, 0x80, 0x00 };
	writeSerialPort( fd, ( uint8_t* )&data, 5 );
	usleep( uSec );
	Stop();
}

void Create::TurnRight()
{
	Backup( 400000 );
	_turning = true;
	_turningTimeout = GetTimeMs() + 2000 + ( rand() % 2000 );
	uint8_t data[5] = { 137, 0, 150, 0x00, 0x01 };
	writeSerialPort( fd, ( uint8_t* )&data, 5 );
}

void Create::TurnLeft()
{
	Backup( 400000 );
	_turning = true;
	_turningTimeout = GetTimeMs() + 2000 + ( rand() % 2000 );
	uint8_t data[5] = { 137, 0, 150, 0xFF, 0xFF };
	writeSerialPort( fd, ( uint8_t* )&data, 5 );
}

void Create::TurnAround()
{
	uint8_t data[5] = { 137, 0, 150, 0xFF, 0xFF };
	writeSerialPort( fd, ( uint8_t* )&data, 5 );
	usleep( 3000000 );
	Stop();
}

void Create::GoHome()
{
	uint8_t data[1] = { 143 };
	writeSerialPort( fd, ( uint8_t* )&data, 1 );
}

bool Create::IsCharged()
{
	UpdateSensors();
	return _chargingState == 2;
}

void Create::UpdateSensors()
{
	if( _updateSensorTimeout < GetTimeMs() )
	{
		uint8_t sensorData[5];
		uint8_t data[6] = { 149, 4, 7, 13, 21, 25 };
		writeSerialPort( fd, ( uint8_t* )&data, 6 );
		readSerialPort( fd, ( uint8_t* )&sensorData, 5 );
		_bumpLeft = ( sensorData[0] & 1 ) != 0;
		_bumpRight = ( sensorData[0] & 2 ) != 0;
		_virtualWall = sensorData[1] != 0;
		_chargingState = sensorData[2];
		_batteryCharge = sensorData[4] + 256 * sensorData[3];
		printf( "Battery level: %d\n", _batteryCharge );
		printf( "Charging State: %d\n", _chargingState );
		_updateSensorTimeout = GetTimeMs() + 20;
	}
}