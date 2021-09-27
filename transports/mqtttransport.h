#ifndef MQTTTRANSPORT_H
#define MQTTTRANSPORT_H

#include "tcptransport.h"

class MqttTransport : public TcpTransport
{
public:
	MqttTransport();

	int setup(const std::string &host, uint16_t port);
	int send(const char *data, int length);
};

#endif // MQTTTRANSPORT_H
