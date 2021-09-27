#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <string>

class Transport
{
public:
	Transport();
	virtual ~Transport() {}

	virtual int setup(const std::string &host, uint16_t port) = 0;
	virtual int send(const char *data, int length) = 0;
};

#endif // TRANSPORT_H
