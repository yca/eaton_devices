#ifndef HUMIDITYMEASUREMENT_H
#define HUMIDITYMEASUREMENT_H

#include <measurement.h>

class HumidityMeasurement : public Measurement
{
public:
	HumidityMeasurement(int64_t ts, double v);

	const std::string name() const { return "humidity"; };
	const std::string unit() const { return ""; };
	virtual Measurements type() const { return Measurement::HUMIDTY; };
};

#endif // HUMIDITYMEASUREMENT_H
