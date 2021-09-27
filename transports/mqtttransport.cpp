#include "mqtttransport.h"

#include <vector>
#include <cstring>

MqttTransport::MqttTransport()
{

}

int MqttTransport::setup(const std::string &host, uint16_t port)
{
	return TcpTransport::setup(host, port);
}

int MqttTransport::send(const char *data, int length)
{
	std::vector<char> vec;
	if (length < 127) {
		vec.resize(length + 2);
		memcpy(vec.data() + 2, data, length);
	} else {
		vec.resize(length + 3);
		memcpy(vec.data() + 3, data, length);
	}
	return TcpTransport::send(vec.data(), vec.size());
}
