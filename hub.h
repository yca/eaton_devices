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

protected:
	void printInfo();
	void processMessage(const std::string &mes, const std::string &uuid);

	HubPriv *p;
};

#endif // HUB_H
