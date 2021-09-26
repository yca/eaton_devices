#ifndef PRESSURESENSOR_H
#define PRESSURESENSOR_H

#include <device.h>

class PressureSensor : public Device
{
public:
	PressureSensor(Transport *transport);

	void takeMeasurements();
	void sendMeasurements();
};

#endif // PRESSURESENSOR_H
