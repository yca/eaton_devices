#include "hub.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/enable_shared_from_this.hpp>

using boost::asio::ip::tcp;

class TcpConnection : public boost::enable_shared_from_this<TcpConnection>
{
public:
	typedef boost::shared_ptr<TcpConnection> pointer;

	static pointer create(boost::asio::io_context & io_ctx)
	{
		return pointer(new TcpConnection(io_ctx));
	}

	tcp::socket& socket()
	{
		return sock;
	}

	void start()
	{
		//message_ = make_daytime_string();

		boost::asio::async_write(sock, boost::asio::buffer(message),
								 boost::bind(&TcpConnection::handle_write, shared_from_this(),
											 boost::asio::placeholders::error,
											 boost::asio::placeholders::bytes_transferred));
	}

private:
	TcpConnection(boost::asio::io_context& io_ctx)
		: sock(io_ctx)
	{
	}

	void handle_write(const boost::system::error_code& /*error*/,
					  size_t /*bytes_transferred*/)
	{
	}

	tcp::socket sock;
	std::string message;
};

class TcpServer
{
public:
	TcpServer(int port, boost::asio::io_context &ctx)
		: ioctx(ctx),
		  acc(ctx, tcp::endpoint(tcp::v4(), port))
	{
		startAccept();
	}

protected:
	void startAccept()
	{
		TcpConnection::pointer new_connection =
			  TcpConnection::create(ioctx);

			acc.async_accept(new_connection->socket(),
				boost::bind(&TcpServer::handleAccept, this, new_connection,
				  boost::asio::placeholders::error));
	}

	void handleAccept(TcpConnection::pointer newConnection,
		  const boost::system::error_code& error)
	{
		if (!error)
			newConnection->start();

		startAccept();
	}

	boost::asio::io_context &ioctx;
	tcp::acceptor acc;
};

class HubPriv
{
public:
	boost::asio::io_context ctx;
	TcpServer *tcpServer = nullptr;
};

Hub::Hub()
{
	p = new HubPriv;
}

int Hub::startTcp(int port)
{
	p->tcpServer = new TcpServer(port, p->ctx);
	p->ctx.run();
	return 0;
}
