#include "uuid.h"
#include "device.h"
#include "transport.h"
#include "hub.h"
#include "arguments.h"

#include "3rdparty/loguru/debug.h"

#include "devices/ambientsensor.h"
#include "devices/pressuresensor.h"

#include "transports/udptransport.h"
#include "transports/tcptransport.h"
#include "transports/mqtttransport.h"

#include <sockpp/socket.h>

#include <thread>

#include <assert.h>
#include <ncurses.h>

const uint16_t SERVER_PORT = 21569;

/* crash handling related */
typedef void (*sighandler)(int, siginfo_t *, void *);
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <cxxabi.h>

class Simulated
{
public:
	enum TransportType {
		TRANSPORT_UDP,
		TRANSPORT_TCP,
		TRANSPORT_MQTT,
	};

	Simulated(int deviceCount, TransportType ttype, bool names)
	{
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_int_distribution<std::mt19937::result_type> dist(1, 2);
		for (int i = 0; i < deviceCount; i++) {
			gLogV("creating device transport %d", i);
			auto *transport = createTransport(ttype);
			assert(transport != nullptr);

			gLog("creating device %d", i);
			auto devt = dist(rng);
			Device *dev = nullptr;
			if (devt == 1)
				dev = new AmbientSensor(transport);
			else if (devt == 2)
				dev = new PressureSensor(transport);
			else
				dev = new AmbientSensor(transport);
			dev->sendDeviceName(names);
			devices.push_back(dev);

		}
	}

	int runSimulation(int samplingPeriodMs, int latencyMs)
	{
		gWarn("starting main sim loop");
		std::chrono::milliseconds passed;
		auto samplingPeriod = std::chrono::milliseconds(samplingPeriodMs);
		while (1) {
			std::this_thread::sleep_for(samplingPeriod);
			for (auto *dev: devices)
				dev->takeMeasurements();
			passed += samplingPeriod;
			if (passed >= std::chrono::milliseconds(latencyMs)) {
				passed = std::chrono::milliseconds();
				for (auto *dev: devices)
					dev->sendMeasurements();
			}
		}

		return 0;
	}

protected:
	static Transport *createTransport(TransportType ttype)
	{
		Transport *t = nullptr;
		if (ttype == TRANSPORT_UDP)
			t = new UdpTransport();
		else if (ttype == TRANSPORT_TCP)
			t = new TcpTransport();
		else if (ttype == TRANSPORT_MQTT)
			t = new MqttTransport();

		int err = t->setup("127.0.0.1", SERVER_PORT);
		if (err)
			gWarn("Error connecting to '%s:%d'", "127.0.0.1", SERVER_PORT);

		return t;
	}

protected:
	std::vector<Device *> devices;
};

static inline void install_sahandler(sighandler signal_handler)
{
	struct sigaction sig_action;
	memset(&sig_action, 0, sizeof(sig_action));
	sigemptyset(&sig_action.sa_mask);
	sig_action.sa_flags |= SA_SIGINFO;
	sig_action.sa_sigaction = signal_handler;
	sigaction(SIGTERM, &sig_action, NULL);
	sigaction(SIGABRT, &sig_action, NULL);
	sigaction(SIGBUS, &sig_action, NULL);
	sigaction(SIGFPE, &sig_action, NULL);
	sigaction(SIGINT, &sig_action, NULL);
	sigaction(SIGSEGV, &sig_action, NULL);
}

static inline void unwind_backtrace()
{
	unw_cursor_t cursor;
	unw_context_t context;

	unw_getcontext(&context);
	unw_init_local(&cursor, &context);

	int n = 0;
	while (unw_step(&cursor)) {
		unw_word_t ip, sp, off;

		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		unw_get_reg(&cursor, UNW_REG_SP, &sp);

		char symbol[256] = {"<unknown>"};
		char *name = symbol;
		std::string line = "";
		if (!unw_get_proc_name(&cursor, symbol, sizeof(symbol), &off)) {
			int status;

			if ((name = abi::__cxa_demangle(symbol, NULL, NULL, &status)) == 0)
				name = symbol;
		}

		fprintf(stderr, "#%-2d 0x%016" PRIxPTR " sp=0x%016" PRIxPTR " %s + 0x%" PRIxPTR
			   "\r\n",
			   ++n, static_cast<uintptr_t>(ip), static_cast<uintptr_t>(sp),
			   name, static_cast<uintptr_t>(off));

		if (name != symbol)
			free(name);
	}
}

int main(int argc, char *argv[])
{
	if (argumentExist("--help")) {
		printf("--devices:	[int] Select number of devices to create for simulation. (default=10)\n");
		printf("--sampling:	[int] Select the sensor sampling interval in milliseconds. (default=10)\n");
		printf("--latency:	[int] Select communication latency in milliseconds. (default=50)\n");
		printf("--udp:		[none] Enables UDP transport for the simulation.\n");
		printf("--mqtt:		[none] Enables MQTT transport for the simulation.\n");
		printf("--no-stats:	[none] Do not print ncurses statistics, helpful during debugging.\n");
		printf("--names:	[none] Show device names instead of endpoint addresses.\n");
		return 0;
	}

	/* install a crash handler for getting a reasonable crash dump */
	install_sahandler([](int signo, siginfo_t *, void *){
		fprintf(stderr, "Received signal %d\n", signo);

		/* ncurses: leave console in a good state */
		endwin();

		/* print application stack trace in case of crashes */
		if (signo != SIGINT)
			unwind_backtrace();

		/* exit with a meaningful error code */
		if (signo == SIGINT)
			exit(0);
		else
			exit(1);
	});

	/* our default verbosity is to print only gWarn statements */
	loguru::g_stderr_verbosity = 3;
	/* override default verbosity if we have 'DEBUG' env var set */
	char *var = getenv("DEBUG");
	if (var)
		loguru::g_stderr_verbosity = std::stoi(var);

	/* init our socket library */
	sockpp::socket_initializer sockInit;

	/* TODO: add command line parameters */
	int devcnt = 10;
	if (argumentExist("--devices"))
		devcnt = std::atoi(argumentGet("--devices"));
	int sampms = 10;
	if (argumentExist("--sampling"))
		sampms = std::atoi(argumentGet("--sampling"));
	int latms = 50;
	if (argumentExist("--latency"))
		latms = std::atoi(argumentGet("--latency"));
	Simulated::TransportType ttype = Simulated::TRANSPORT_TCP;
	if (argumentExist("--udp"))
		ttype = Simulated::TRANSPORT_UDP;
	if (argumentExist("--mqtt"))
		ttype = Simulated::TRANSPORT_MQTT;
	bool statsEnabled = true;
	if (argumentExist("--no-stats"))
		statsEnabled = false;
	bool useDeviceNames = false;
	if (argumentExist("--names"))
		useDeviceNames = true;

	gWarn("Initializing local hub");
	/* start log collection using hub instance */
	Hub hub;
	hub.showStats(statsEnabled);
	hub.enableDeviceNameReading(useDeviceNames);
	int err = -ENOENT;
	if (ttype == Simulated::TRANSPORT_TCP)
		err = hub.startTcp(SERVER_PORT);
	else if (ttype == Simulated::TRANSPORT_UDP)
		err = hub.startUdp(SERVER_PORT);
	else if (ttype == Simulated::TRANSPORT_MQTT)
		err = hub.startMqtt(SERVER_PORT);
	if (err) {
		gWarn("Error '%d' starting hub, aborting simulation.", err);
		return err;
	}

	gWarn("starting simulation");
	/* create and start simulation */
	Simulated sim(devcnt, ttype, useDeviceNames);
	return sim.runSimulation(sampms, latms);
}
