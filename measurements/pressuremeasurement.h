#ifndef PRESSUREMEASUREMENT_H
#define PRESSUREMEASUREMENT_H

#include <measurement.h>

class PressureMeasurement : public Measurement
{
public:
	PressureMeasurement(int64_t ts, double v);

	const std::string name() const { return "pressure"; };
	const std::string unit() const { return "atm"; };
	virtual Measurements type() const { return Measurement::PRESSURE; };
};

#endif // PRESSUREMEASUREMENT_H
