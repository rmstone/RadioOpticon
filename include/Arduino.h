#ifndef ARDUINO_H
#define ARDUINO_H
#include <string>

enum SensorState
{
	clear = 0,
	personFound,
};

#define TEMP_MARGIN 5

class Arduino
{
public:
	Arduino( std::string portname );
	~Arduino();

	bool isValid();

	SensorState CheckSensors();
	void ClearFound();
private:
	int fd;
};

#endif