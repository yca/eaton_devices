#ifndef TEMPERATUREMEASUREMENT_H
#define TEMPERATUREMEASUREMENT_H

#include <measurement.h>

class TemperatureMeasurement : public Measurement
{
public:
	TemperatureMeasurement(int64_t ts, double v);

	const std::string name() const { return "temperature"; };
	const std::string unit() const { return "celcius"; };
	virtual Measurements type() const { return Measurement::TEMPERATURE; };
};

#endif // TEMPERATUREMEASUREMENT_H
