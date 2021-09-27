#ifndef TCPTRANSPORT_H
#define TCPTRANSPORT_H

#include <transport.h>

#include <string>

class TcpTransportPriv;

class TcpTransport : public Transport
{
public:
	TcpTransport();
	~TcpTransport();

	int setup(const std::string &host, uint16_t port);
	int send(const char *data, int length);

protected:
	TcpTransportPriv *p;
};

#endif // TCPTRANSPORT_H
