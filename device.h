#ifndef DEVICE_H
#define DEVICE_H

#include <transport.h>

#include <sstream>

class Device
{
public:
	Device(Transport *transport);
	virtual ~Device() {}

	virtual void sendMeasurements() = 0;

protected:
	/* some helper functions */
	static void writeMeasurement(std::stringstream &ss, int32_t type, float value);

protected:
	Device() = delete;
	Transport *tr;
};

#endif // DEVICE_H
