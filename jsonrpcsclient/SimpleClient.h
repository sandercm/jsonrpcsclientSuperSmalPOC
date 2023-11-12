#pragma once
#include <boost/asio.hpp>
#include <boost/asio/experimental/as_tuple.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#define BOOST_CONTAINER_NO_LIB 
#define BOOST_JSON_NO_LIB
#include <boost/json.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;
using namespace boost::json;
class SimpleClient
{
private:
	tcp::socket& m_socket;
	const std::string& m_host;
	const std::string& m_port;
	stream_parser m_parser;

	std::string m_partialString;
public:
	SimpleClient(tcp::socket& socket, const std::string& host, const std::string& port);
	awaitable<void> Connect();
	awaitable<void> Read();
	awaitable<void> Write(const std::vector<char>& data);
private:
	awaitable<void> _Connect();
};

