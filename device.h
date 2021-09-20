#ifndef DEVICE_H
#define DEVICE_H

#include <transport.h>

class Device
{
public:
	Device(Transport *transport);
	virtual ~Device() {}

	virtual void sendMeasurements() = 0;

protected:
	Device() = delete;
	Transport *tr;
};

#endif // DEVICE_H
