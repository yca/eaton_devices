#ifndef TEMPERATUREMEASUREMENT_H
#define TEMPERATUREMEASUREMENT_H

#include <measurement.h>

class TemperatureMeasurement : public Measurement
{
public:
	TemperatureMeasurement(int64_t ts, double v);

	const std::string name() const { return "temperature"; };
};

#endif // TEMPERATUREMEASUREMENT_H
