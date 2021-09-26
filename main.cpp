#include "uuid.h"
#include "device.h"
#include "transport.h"
#include "3rdparty/loguru/debug.h"
#include "hub.h"

#include "transports/udptransport.h"
#include "transports/tcptransport.h"
#include "transports/mqtttransport.h"

#include <thread>

class Simulated
{
public:
	enum TransportType {
		TRANSPORT_UDP,
		TRANSPORT_TCP,
		TRANSPORT_MQTT,
	};

	Simulated(int deviceCount, TransportType ttype)
	{
		for (int i = 0; i < deviceCount; i++) {
			Device *dev = createRandomDevice(ttype);
			devices.push_back(dev);
		}
	}

	int runSimulation(int latencyMs)
	{
		while (1) {
			std::this_thread::sleep_for(std::chrono::milliseconds(latencyMs));
			for (auto *dev: devices)
				dev->sendMeasurements();
		}

		return 0;
	}

protected:
	Device * createRandomDevice(TransportType ttype)
	{
		/*if (ttype == TRANSPORT_UDP)
			dev = new Device(new UdpTransport);
		else if (ttype == TRANSPORT_TCP)
			dev = new Device(new UdpTransport);*/
		return nullptr;
	}

protected:
	std::vector<Device *> devices;
};

int main()
{
	/* our default verbosity is to print only gWarn statements */
	loguru::g_stderr_verbosity = 3;
	/* override default verbosity if we have 'DEBUG' env var set */
	char *var = getenv("DEBUG");
	if (var)
		loguru::g_stderr_verbosity = std::stoi(var);

	/* TODO: add command line parameters */
	int devcnt = 100;
	int latms = 50;
	Simulated::TransportType ttype = Simulated::TRANSPORT_MQTT;

	/* start log collection using hub instance */
	Hub hub;
	int err = hub.startTcp(21569);
	if (err) {
		gWarn("Error '%d' starting hub, aborting simulation.", err);
		return err;
	}

	/* create and start simulation */
	Simulated sim(devcnt, ttype);
	return sim.runSimulation(latms);
}
