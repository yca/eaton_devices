#ifndef TRANSPORT_H
#define TRANSPORT_H

class Transport
{
public:
	Transport();
	virtual ~Transport() {}

	virtual int send(const char *data, int length) = 0;
};

#endif // TRANSPORT_H
