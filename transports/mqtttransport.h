#ifndef MQTTTRANSPORT_H
#define MQTTTRANSPORT_H

#include <transport.h>

class MqttTransport : public Transport
{
public:
	MqttTransport();

	int setup();
	int send(const char *data, int length);
};

#endif // MQTTTRANSPORT_H
