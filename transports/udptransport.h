#ifndef UDPTRANSPORT_H
#define UDPTRANSPORT_H

#include <transport.h>

#include <string>

class UdpTransportPriv;

class UdpTransport : public Transport
{
public:
	UdpTransport();
	~UdpTransport();

	int setup(const std::string &host, uint16_t port);
	int send(const char *data, int length);

protected:
	UdpTransportPriv *p;
};

#endif // UDPTRANSPORT_H
