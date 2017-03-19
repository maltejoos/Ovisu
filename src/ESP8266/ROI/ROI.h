/*
 ROI.h - Roomba Open Interface class
  Malte Joos
*/

#ifndef ROI_h
#define ROI_h

#include <SoftwareSerial.h>

class ROI
{
public:
	ROI(SoftwareSerial& softserial);

	void begin();

	int getOIMode();

	int getDeltaTranslationMm();
	int getDeltaRotationDeg();

	void startClean();
	void seekDock();

	int isCharging();

	int getBatteryPct();

private:
	SoftwareSerial* p_serial;
};

#endif
