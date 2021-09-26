#include "hub.h"
#include "timing.h"
#include "measurement.h"

#include "measurements/humiditymeasurement.h"
#include "measurements/pressuremeasurement.h"
#include "measurements/temperaturemeasurement.h"

#include <sockpp/tcp_socket.h>
#include <sockpp/tcp_acceptor.h>

#include <3rdparty/loguru/debug.h>

#include <ncurses.h>

#include <thread>
#include <unordered_map>

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

class HubPriv
{
public:
	TcpServer *tcpServer = nullptr;
	std::mutex m;
	std::vector<Measurement *> all;
	Timing t;
	size_t mesCount = 0;
	size_t bytesTotal = 0;
	size_t payloadTotal = 0;
	std::unordered_map<std::string, int> typeCount;
	std::unordered_map<std::string, int> latency;
	std::unordered_map<std::string, Timing *> timers;
	int printIntervalMsec = 3000;
};

Hub::Hub()
{
	p = new HubPriv;
}

int Hub::startTcp(int port)
{
	p->t.start();
	p->tcpServer = new TcpServer(port, [this](const auto &mes, auto &peerSock){
		gLogV("new %ld bytes message from '%s'", mes.size(),
			  peerSock.peer_address().to_string().data());

		std::string uuid = peerSock.peer_address().to_string();

		std::stringstream ss(mes);
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

			/* bandwidth */

			/*
			 * bandwidth and overhead calculation:
			 *	TCP header: 20 bytes (min)
			 *	IP header: 20 bytes
			 *	Ethernet header: 14 bytes
			 *	some padding (min packet size is 64 bytes on the wire)
			 */
			int totalLen = mes.size() + 54;
			if (totalLen < 64)
				totalLen = 64;
			p->payloadTotal += mes.size();
			p->bytesTotal += totalLen;
		}
		p->m.unlock();

		if (p->t.elapsedMili() > p->printIntervalMsec) {
			p->t.restart();
			printInfo();
		}
	});
	return 0;
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
	printw("we have %ld sensor connections\n", p->tcpServer->connectionCount());
	printw("recved %ld messages up to now\n", p->mesCount);
	for (const auto &[key, value]: p->typeCount)
		printw("\t%ld %s messages\n", value, key.data());
	printw("recved %ld messages in the last 3 seconds\n", p->printIntervalMsec / 1000);
	printw("average latency is calculated as %d milliseconds\n", lavg);
	printw("total bandwidth is is calculated as %.2lf kbits / sec\n", double(bps) / 1000.0);
	printw("communication overhead is calculated as %d\%\n", overhead);

	refresh();
}
