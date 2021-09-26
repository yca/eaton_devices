#ifndef DEVICE_H
#define DEVICE_H

#include <transport.h>

#include <sstream>

class Device
{
public:
	Device(Transport *transport);
	virtual ~Device() {}

	virtual void takeMeasurements() = 0;
	void sendMeasurements();

protected:
	/* some helper functions */
	static void writeMeasurement(std::stringstream &ss, int32_t type, float value);

protected:
	Device() = delete;

	Transport *tr;
	std::stringstream ss;
};

#endif // DEVICE_H
