#ifndef HUB_H
#define HUB_H

class HubPriv;

class Hub
{
public:
	Hub();

	int startTcp(int port);

protected:
	void printInfo();

	HubPriv *p;
};

#endif // HUB_H
