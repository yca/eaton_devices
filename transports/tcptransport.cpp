#include "tcptransport.h"

#include "sockpp/tcp_connector.h"

class TcpTransportPriv
{
public:
	sockpp::tcp_connector conn;
};

TcpTransport::TcpTransport()
{
	p = new TcpTransportPriv;
}

TcpTransport::~TcpTransport()
{
	delete p;
}

int TcpTransport::setup(const std::string &host, uint16_t port)
{
	p->conn = sockpp::tcp_connector({host, port});
	p->conn.read_timeout(std::chrono::milliseconds(500));
	if (p->conn.is_connected())
		return 0;
	return -EIO;
}

int TcpTransport::send(const char *data, int length)
{
	return p->conn.write(data, length);
}
