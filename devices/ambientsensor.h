#ifndef AMBIENTSENSOR_H
#define AMBIENTSENSOR_H

#include <device.h>

class AmbientSensor : public Device
{
public:
	AmbientSensor(Transport *transport);

	void takeMeasurements();
};

#endif // AMBIENTSENSOR_H
