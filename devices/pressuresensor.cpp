#include "pressuresensor.h"
#include "measurement.h"

#include <3rdparty/loguru/debug.h>

#include <string>
#include <sstream>

PressureSensor::PressureSensor(Transport *transport)
	: Device(transport)
{

}

void PressureSensor::sendMeasurements()
{
	std::stringstream ss;
	writeMeasurement(ss, Measurement::PRESSURE, 5.0);
	const auto &mes = ss.str();
	tr->send(mes.data(), mes.size());
}
