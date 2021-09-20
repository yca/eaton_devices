#include "debug.h"

#ifdef LOGURU_WITH_UDP
#include <utils/network.h>
#endif

#ifdef LOGURU_WITH_UDP
static utils::network::UdpSocket udpcli;

static void debug_logudp(void*, const loguru::Message& message)
{
	std::string mes;
	mes.reserve(strlen(message.preamble) + strlen(message.indentation) + strlen(message.prefix)
				+ strlen(message.message) + 1);
	mes.append(message.preamble);
	mes.append(message.indentation);
	mes.append(message.prefix);
	mes.append(message.message);
	mes.append("\n");
	udpcli.send(mes);
}
#endif

void debugEnableUDPLogging(std::string target, int16_t port)
{
#ifdef LOGURU_WITH_UDP
	printf("Enabling UDP logging to '%s:%d'\n", target.data(), port); fflush(stdout);
	loguru::add_callback("networklog", debug_logudp, nullptr, loguru::Verbosity_MAX);
	udpcli.connect(target, port);
#endif
}

