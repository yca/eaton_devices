#include "hub.h"
#include "uuid.h"
#include "timing.h"
#include "measurement.h"

#include "measurements/humiditymeasurement.h"
#include "measurements/pressuremeasurement.h"
#include "measurements/temperaturemeasurement.h"

#include <sockpp/tcp_socket.h>
#include <sockpp/udp_socket.h>
#include <sockpp/tcp_acceptor.h>

#include <3rdparty/loguru/debug.h>

#include <ncurses.h>

#include <thread>
#include <unordered_map>
#include <unordered_set>

class TcpConnection
{
public:
	TcpConnection(sockpp::tcp_socket &&s, std::function<void(const std::string &, sockpp::tcp_socket  &)> readCallback)
	 : sock(std::move(s))
	{
		rcb = readCallback;
		finished = false;
		/* we set an infinite timeout */
		sock.read_timeout(std::chrono::microseconds(0));
		readThread = std::thread([this](){
			char readbuf[4096];
			while (1) {
				gLogVVV("new message from '%s'", sock.peer_address().to_string().data());
				int len = sock.read(readbuf, sizeof(readbuf));
				if (len < 0) {
					/* connection closed */
					finished = true;
					break;
				}
				if (rcb)
					rcb(std::string(readbuf, len), sock);
			}
		});
	}

	~TcpConnection()
	{
		readThread.join();
	}

	bool isFinished()
	{
		return finished;
	}

protected:
	bool finished;
	std::thread readThread;
	sockpp::tcp_socket sock;
	std::function<void(const std::string &message, sockpp::tcp_socket &peerSocket)> rcb;
};

class TcpServer
{
public:
	TcpServer(int port, std::function<void(const std::string &, sockpp::tcp_socket &)> readCallback)
		: acc(port)
	{
		rcb = readCallback;
		thr = std::thread([this](){
			while (1) {
				sockpp::inet_address peer;
				sockpp::tcp_socket sock = acc.accept(&peer);
				if (sock) {
					gWarn("Accepted new incoming connection request from '%s'",
						  sock.peer_address().to_string().data());
					m.lock();
					connections.push_back(new TcpConnection(std::move(sock), [this](const std::string &mes,
															sockpp::tcp_socket &peerSocket){
						if (rcb)
							rcb(mes, peerSocket);
					}));
					m.unlock();
				}
			}
		});
	}

	size_t connectionCount()
	{
		std::unique_lock<std::mutex> l(m);
		return connections.size();
	}

	void wait()
	{
		thr.join();
	}


protected:
	std::thread thr;
	std::mutex m;
	sockpp::tcp_acceptor acc;
	std::vector<TcpConnection *> connections;
	std::function<void(std::string, sockpp::tcp_socket &)> rcb;
};

class UdpServer
{
public:
	UdpServer(int port, std::function<void(const std::string &, sockpp::udp_socket::addr_t &)>
			  readCallback)
	{
		rcb = readCallback;
		if (!sock.bind(sockpp::inet_address("127.0.0.1", port))) {
			gWarn("error binding to UDP port %d", port);
			return;
		}
		thr = std::thread([this, port](){
			char buf[4096];
			while (1) {
				sockpp::udp_socket::addr_t addr;
				int len = sock.recv_from(buf, sizeof(buf), &addr);
				if (rcb && len > 0)
					rcb(std::string(buf, len), addr);
				if (!addr.to_string().empty())
					peers.insert(addr.to_string());
			}
		});
		running = true;
	}

	bool isRunning() { return running; }

	size_t peerCount()
	{
		return peers.size();
	}

protected:
	bool running = false;
	std::thread thr;
	sockpp::udp_socket sock;
	std::unordered_set<std::string> peers;
	std::function<void(std::string, sockpp::udp_socket::addr_t &)> rcb;
};

class HubPriv
{
public:
	TcpServer *tcpServer = nullptr;
	TcpServer *mqttServer = nullptr;
	UdpServer *udpServer = nullptr;
	std::mutex m;
	std::vector<Measurement *> all;
	Timing t;
	size_t mesCount = 0;
	size_t bytesTotal = 0;
	size_t payloadTotal = 0;
	std::unordered_map<std::string, int> typeCount;
	std::unordered_map<std::string, int> latency;
	std::unordered_map<std::string, Timing *> timers;
	std::unordered_map<std::string, int> messageCountByDev;
	int printIntervalMsec = 3000;
	bool printStats = true;
	bool readDeviceNames = false;
	std::string uuid;
};

Hub::Hub()
{
	p = new HubPriv;
	p->uuid = uuid::generate_uuid_v4();
}

Hub::~Hub()
{
	if (p->tcpServer)
		delete p->tcpServer;
	if (p->udpServer)
		delete p->udpServer;
	if (p->mqttServer)
		delete p->mqttServer;
	for (auto &[k, t]: p->timers)
		delete t;
	delete p;
}

void Hub::showStats(bool enable)
{
	p->printStats = enable;
}

int Hub::startTcp(int port)
{
	p->t.start();
	p->tcpServer = new TcpServer(port, [this](const auto &mes, auto &peerSock){
		gLogV("new %ld bytes message from '%s'", mes.size(),
			  peerSock.peer_address().to_string().data());
		processMessage(mes, peerSock.peer_address().to_string());
	});
	return 0;
}

int Hub::startUdp(int port)
{
	p->t.start();
	p->udpServer = new UdpServer(port, [this](const auto &mes, auto &addr){
		gLog("new %ld bytes message from '%s'", mes.size(),
			  addr.to_string().data());
		processMessage(mes, addr.to_string());
	});
	if (p->udpServer->isRunning())
		return 0;
	return -1;
}

int Hub::startMqtt(int port)
{
	p->t.start();
	p->mqttServer = new TcpServer(port, [this](const auto &mes, auto &peerSock){
		gLogV("new %ld bytes message from '%s'", mes.size(),
			  peerSock.peer_address().to_string().data());
		std::string uuid = peerSock.peer_address().to_string();
		processMessage(mes, uuid);
	});
	return 0;
}

void Hub::enableDeviceNameReading(bool v)
{
	p->readDeviceNames = v;
}

void Hub::printInfo()
{
	p->m.lock();
	int bps = 0;
	size_t total = 0;
	int overhead = 0;
	for (const auto *m: p->all) {
		total++;
		p->typeCount[m->name()]++;
		delete m;
	}
	size_t lavg = 0;
	for (const auto & [key, value]: p->latency)
		lavg += value;
	lavg /= p->latency.size();
	p->all.clear();
	bps = int((double)p->bytesTotal * 8 / p->printIntervalMsec * 1000);
	overhead = 100 - 100 * p->payloadTotal / p->bytesTotal;
	p->bytesTotal = 0;
	p->payloadTotal = 0;
	p->m.unlock();
	p->mesCount += total;

	/* print stats */
	initscr();
	clear();
	if (p->tcpServer) {
		printw("transport type is TCP\n");
		printw("we have %ld sensor connections\n", p->tcpServer->connectionCount());
	} else if (p->udpServer) {
		printw("transport type is UDP\n");
		printw("we have %ld sensor connections\n", p->udpServer->peerCount());
	} else if (p->mqttServer) {
		printw("transport type is MQTT\n");
		printw("we have %ld sensor connections\n", p->mqttServer->connectionCount());
	}
	printw("recved %ld messages up to now\n", p->mesCount);
	for (const auto &[key, value]: p->typeCount)
		printw("\t%ld %s messages\n", value, key.data());
	printw("recved %ld messages in the last %ld seconds\n", total, p->printIntervalMsec / 1000);
	printw("average latency is calculated as %d milliseconds\n", lavg);
	printw("total bandwidth is is calculated as %.2lf kbits / sec\n", double(bps) / 1000.0);
	printw("communication overhead is calculated as %d\%\n", overhead);
	printw("message count by device:\n");
	for (const auto &[key, value]: p->messageCountByDev)
		printw("\t%s: %ld messages\n", key.data(), value);

	refresh();
}

void Hub::processMessage(const std::string &mes, const std::string &addr)
{
	std::stringstream ss(mes);
	if (p->mqttServer && mes.size() <= 128)
		ss.seekg(2);
	else if (p->mqttServer)
		ss.seekg(3);
	std::string uuid;
	uuid.resize(p->uuid.size());
	if (p->readDeviceNames)
		ss.read(uuid.data(), p->uuid.size());
	else
		uuid = addr;
	while (!ss.eof()) {
		int32_t type = 0;
		float value = 0;
		ss.read(reinterpret_cast<char *>(&type), sizeof(int32_t));
		if (ss.eof())
			break;
		ss.read(reinterpret_cast<char *>(&value), sizeof(float));
		if (ss.eof())
			break;

		int64_t ts = 0;
		p->m.lock();
		if (type == Measurement::HUMIDTY)
			p->all.push_back(new HumidityMeasurement(ts, value));
		else if (type == Measurement::PRESSURE)
			p->all.push_back(new PressureMeasurement(ts, value));
		else if (type == Measurement::TEMPERATURE)
			p->all.push_back(new TemperatureMeasurement(ts, value));
		p->messageCountByDev[uuid]++;
		p->m.unlock();
	}

	/* measure statistics */
	p->m.lock();
	{
		/* latency */
		if (!p->timers[uuid])
			p->timers[uuid] = new Timing;
		auto *timing = p->timers[uuid];
		p->latency[uuid] = timing->elapsedMili();
		timing->restart();

		auto overheadHelper = [&](int hdrSize){
			int totalLen = mes.size() + hdrSize;
			if (totalLen < 64)
				totalLen = 64;
			p->payloadTotal += mes.size();
			if (p->readDeviceNames)
				p->payloadTotal -= p->uuid.size();
			p->bytesTotal += totalLen;
		};

		/* bandwidth and overhead calculation */
		if (p->tcpServer) {
			/*
			 *	TCP header: 20 bytes (min)
			 *	IP header: 20 bytes
			 *	Ethernet header: 14 bytes
			 *	some padding (min packet size is 64 bytes on the wire)
			 */
			overheadHelper(54);
			/*
			 * although we're only recv'ing, so we need to generate
			 *	an ACK message for every 2 packets, so we add 32 bytes to
			 *	each message for that.
			 */
			p->bytesTotal += 32;
		} else if (p->udpServer) {
			/*
			 *	UDP header: 8 bytes
			 *	IP header: 20 bytes
			 *	Ethernet header: 14 bytes
			 *	some padding (min packet size is 64 bytes on the wire)
			 */
			overheadHelper(42);
		} else if (p->mqttServer) {
			/*
			 *  MQTT header: 2 or 3 bytes
			 *	TCP header: 20 bytes (min)
			 *	IP header: 20 bytes
			 *	Ethernet header: 14 bytes
			 *	some padding (min packet size is 64 bytes on the wire)
			 */
			if (mes.size() <= 128)
				overheadHelper(56);
			else
				overheadHelper(57);
			/*
			 * although we're only recv'ing, so we need to generate
			 *	an ACK message for every 2 packets, so we add 32 bytes to
			 *	each message for that.
			 */
			p->bytesTotal += 32;
		}
	}
	p->m.unlock();

	if (p->t.elapsedMili() > p->printIntervalMsec) {
		p->t.restart();
		if (p->printStats)
			printInfo();
	}
}
