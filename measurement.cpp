#include "measurement.h"

Measurement::Measurement(int64_t ts, double v)
{
	this->v = v;
	this->ts = ts;
}

const double &Measurement::value() const
{
	return v;
}

const int64_t &Measurement::timestamp() const
{
	return ts;
}
