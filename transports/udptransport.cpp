#include "udptransport.h"

#include "sockpp/udp_socket.h"

class UdpTransportPriv
{
public:
	sockpp::udp_socket sock;
};

UdpTransport::UdpTransport()
{
	p = new UdpTransportPriv;
}

UdpTransport::~UdpTransport()
{
	delete p;
}

int UdpTransport::setup(const std::string &host, uint16_t port)
{
	if (p->sock.connect(sockpp::inet_address(host, port)))
		return 0;
	return -1;
}

int UdpTransport::send(const char *data, int length)
{
	const char *buf = data;
	int left = length;
	while (left) {
		int n = p->sock.send((const void *)data, length);
		if (n < 0)
			return -EIO;
		left -= n;
		buf += n;
	}
	return length;
}
