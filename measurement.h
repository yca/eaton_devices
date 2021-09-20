#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <string>

class Measurement
{
public:
	Measurement(int64_t ts, double v);
	virtual ~Measurement() {}

	virtual const std::string name() const = 0;
	const double & value() const;
	const int64_t & timestamp() const;

protected:
	Measurement() = delete;

	double v;
	int64_t ts;
};

#endif // MEASUREMENT_H
