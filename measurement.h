#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <string>

class Measurement
{
public:
	enum Measurements {
		PRESSURE = 0,
		TEMPERATURE = 1,
		HUMIDTY = 2,
	};
	Measurement(int64_t ts, double v);
	virtual ~Measurement() {}

	virtual const std::string name() const = 0;
	virtual Measurements type() const = 0;
	virtual const std::string unit() const = 0;
	const double & value() const;
	const int64_t & timestamp() const;

protected:
	Measurement() = delete;

	double v;
	int64_t ts;
};

#endif // MEASUREMENT_H
