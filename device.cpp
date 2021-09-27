#include "device.h"
#include "uuid.h"

#include "3rdparty/loguru/debug.h"

Device::Device(Transport *transport)
{
	tr = transport;
	uuid = uuid::generate_uuid_v4();
	sendDeviceName(false);
}

void Device::sendMeasurements()
{
	const auto &mes = ss.str();
	tr->send(mes.data(), mes.size());
	sendDeviceName(sendDeviceNameWithMessages);
}

void Device::sendDeviceName(bool v)
{
	ss = std::stringstream();
	sendDeviceNameWithMessages = v;
	if (v)
		ss.write(uuid.data(), uuid.length());
}

void Device::writeMeasurement(std::stringstream &ss, int32_t type, float value)
{
	ss.write(reinterpret_cast<const char *>(&type), sizeof(int32_t));
	ss.write(reinterpret_cast<const char *>(&value), sizeof(float));
}
