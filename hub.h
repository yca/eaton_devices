#ifndef HUB_H
#define HUB_H

#include <string>

class HubPriv;

class Hub
{
public:
	Hub();
	~Hub();

	void showStats(bool enable);
	int startTcp(int port);
	int startUdp(int port);
	int startMqtt(int port);
	void enableDeviceNameReading(bool v);

protected:
	void printInfo();
	void processMessage(const std::string &mes, const std::string &addr);

	HubPriv *p;
};

#endif // HUB_H
