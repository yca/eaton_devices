#include "ambientsensor.h"
#include "measurement.h"

#include <3rdparty/loguru/debug.h>

#include <string>

AmbientSensor::AmbientSensor(Transport *transport)
	: Device(transport)
{

}

void AmbientSensor::sendMeasurements()
{
	std::stringstream ss;
	writeMeasurement(ss, Measurement::TEMPERATURE, 12.5);
	writeMeasurement(ss, Measurement::HUMIDTY, 13.5);
	const auto &mes = ss.str();
	tr->send(mes.data(), mes.size());
}
