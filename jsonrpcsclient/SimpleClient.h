#pragma once
#include <boost/asio.hpp>
#include <boost/asio/experimental/as_tuple.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;

class SimpleClient
{
private:
	tcp::socket& m_socket;
	const std::string& m_host;
	const std::string& m_port;
	bool m_shouldReconnect;
public:
	SimpleClient(tcp::socket& socket, const std::string& host, const std::string& port);
	awaitable<void> Connect();
	awaitable<void> _Connect();
	awaitable<std::string> Read();
	awaitable<void> Write(const std::vector<char>& data);
};

