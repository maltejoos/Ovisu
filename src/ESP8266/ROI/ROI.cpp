#include "ROI.h"
#include <SoftwareSerial.h>
#include "Arduino.h"

void printBin(char c)
{
	char output[9];
	itoa(c, output, 2);
	Serial.print(&output[0]);
}

ROI::ROI(SoftwareSerial& softserial)
{
	p_serial = &softserial;
}

void ROI::begin()
{
	p_serial->write(128);//set Roomba to passiv mode
}

int ROI::getOIMode()
{
	p_serial->write(35);
	char mode = p_serial->read();

	return mode;
}

int ROI::getDeltaTranslationMm()
{
	p_serial->write(128);
	p_serial->write(142);//request sensor data
	p_serial->write(20);//distance. According to different datasheets it should be 19. But in my experiments 20 seems to be the right opcode.

	char high_byte = p_serial->read();
	char low_byte = p_serial->read();

	sint16_t dtrans = (high_byte << 8) + low_byte;

	//somehow forward directions are negative. Invert.
	return -dtrans;
}

int ROI::getDeltaRotationDeg()
{
	p_serial->write(128);
	p_serial->write(142);//request sensor data
	p_serial->write(19);//angle. According to different datasheets it should be 20. But in my experiments 19 seems to be the right opcode.

	char high_byte = p_serial->read();
	char low_byte = p_serial->read();

	sint16_t delta_angle = (high_byte << 8) + low_byte;

	return delta_angle;

}

void ROI::startClean()
{
	p_serial->write(135);//clean
}

void ROI::seekDock()
{
	p_serial->write(143);//seek dock
}

//The ROI of Roomba 782 gets corrupted when writing the opcodes 25 and 26
#if 0
int ROI::getBatteryPct()
{
	p_serial->write(128);
	p_serial->write(142);//request sensor data
	p_serial->write(26);//battery capacity

	char high_byte = p_serial->read();
	char low_byte = p_serial->read();

	uint16_t capacity = (high_byte << 8) + low_byte;

	delay(20);

	p_serial->write(128);
	p_serial->write(142);//request sensor data
	p_serial->write(25);//battery charge

	char high_byte = p_serial->read();
	char low_byte = p_serial->read();

	uint16_t charge = (high_byte << 8) + low_byte;

	int pct = 0;//(((float)(charge))/capacity)*100;

	return pct;
}
#endif

int ROI::isCharging()
{
	p_serial->write(128);

	p_serial->write(142);//request sensor data
	p_serial->write(21);//charging state
	char state = p_serial->read();

	// not charging or error
	if(((int)state == 0) || ((int)state == 5))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
