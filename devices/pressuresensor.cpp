#include "pressuresensor.h"
#include "measurement.h"

#include <3rdparty/loguru/debug.h>

#include <string>
#include <sstream>

PressureSensor::PressureSensor(Transport *transport)
	: Device(transport)
{

}

void PressureSensor::takeMeasurements()
{
	writeMeasurement(ss, Measurement::PRESSURE, 5.0);
}
