#ifndef TCPTRANSPORT_H
#define TCPTRANSPORT_H

#include <transport.h>

class TcpTransport : public Transport
{
public:
	TcpTransport();

	int setup();
	int send(const char *data, int length);
};

#endif // TCPTRANSPORT_H
