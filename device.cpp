#include "device.h"

Device::Device(Transport *transport)
{
	tr = transport;
}

void Device::writeMeasurement(std::stringstream &ss, int32_t type, float value)
{
	ss.write(reinterpret_cast<const char *>(&type), sizeof(int32_t));
	ss.write(reinterpret_cast<const char *>(&value), sizeof(float));
}
