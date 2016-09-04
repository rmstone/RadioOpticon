#ifndef CREATE_H
#define CREATE_H

#include <string>
#include <stdint.h>

#define BATTERY_THRESHOLD 200

class Create
{
public:
	Create( std::string portname );
	~Create();

	bool isValid();
	void DriveAround();
	void Stop();
	void GoHome();
	void TurnAround();
	void UpdateSensors();
	void Driving();
	void Undock();
	bool IsBatteryLow();
	bool IsCharged();


private:
	int fd;
	bool _virtualWall;
	bool _bumpLeft;
	bool _bumpRight;
	bool _wall;
	uint8_t _chargingState;

	uint16_t _batteryCharge;

	uint64_t _turningTimeout;
	uint64_t _updateSensorTimeout;
	bool _turning;

	void DriveForward();
	void TurnLeft();
	void TurnRight();
	void Backup( uint64_t uSec );
};



#endif