#ifndef UDPTRANSPORT_H
#define UDPTRANSPORT_H

#include <transport.h>

class UdpTransport : public Transport
{
public:
	UdpTransport();

	int setup();
	int send(const char *data, int length);
};

#endif // UDPTRANSPORT_H
