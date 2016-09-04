#include <I2C.h>
#include <NewPing.h>
#include <Servo.h>

//I2C pins are 2 SDA & 3 SCL

//Sonar Sensor
#define TRIGGER_PIN        4
#define ECHO_PIN           5
#define MAX_DISTANCE       600

#define SERVO_PIN          6
#define SERVO_CENTER       95
#define SERVO_OFFSET       50

#define SERVO_INC          5

int plus = 1;
int currServo = SERVO_CENTER;
int cm_ping = 0;
uint16_t ambient = 0;
uint8_t found = 0;
uint8_t foundCount = 0;

uint8_t distThreshold = 75;
int8_t tempOffset = 0;
uint16_t foundCountThreshold = 500;

uint8_t data[35];

Servo sonarServo;
NewPing sonar( TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE );

void setup()
{
	Serial.begin(57600);
	sonarServo.attach( SERVO_PIN );
	sonarServo.write( SERVO_CENTER );
	I2c.begin();
}

void loop()
{
	I2c.read(0x0A, 0x4C, 35, (uint8_t*)&data);
	cm_ping = sonar.ping_cm();
	ambient = *(uint16_t *)&data;
	bool foundNow = false;
	if( cm_ping < distThreshold )
	{
		uint16_t *shortData = (uint16_t*)&data;
		for(int i = 1; i < 17; i++)
		{
			if( ( shortData[i] + tempOffset ) > ambient )
			{
				found = 1;
				foundCount = 0;
				foundNow = true;
			}
		}
	}
	if( !foundNow )
	{
		foundCount++;
		if( foundCount > foundCountThreshold )
			found = 0;
	}

	if( Serial.available() )
		serialEvent();
}

void serialEvent()
{
	char inchar = Serial.read();
	switch( inchar )
	{
		case 's':
		{
			Serial.write(found);
			found = 0;
			foundCount = 0;
			break;
		}
		case 't':
		{
			tempOffset = Serial.read();
			break;
		}
		case 'd':
		{
			distThreshold = Serial.read();
			break;
		}
		case 'f':
		{
			foundCountThreshold = Serial.read();
			foundCountThreshold += Serial.read() * 256;
			break;
		}
		case 'c':
		{
			found = 0;
			foundCount = 0;
			break;
		}
	}
	
}
