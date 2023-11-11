#include "SimpleClient.h"
#include <iostream>

SimpleClient::SimpleClient(tcp::socket& socket, const std::string& host, const std::string& port)
	: m_socket(socket)
	, m_host(host)
	, m_port(port)
	, m_shouldReconnect(true)
{
}

awaitable<void> SimpleClient::Connect()
{
	try {
		std::cout << "trying to create Connection" << "\n";
		co_await _Connect();
	}
	catch (const boost::system::system_error& e) {
		std::cout << "Connection error: " << e.what() << "\n";
	}
}

awaitable<void> SimpleClient::_Connect()
{
	tcp::resolver resolver(m_socket.get_executor());
	tcp::resolver::results_type endpoints = co_await resolver.async_resolve(m_host, m_port, use_awaitable);

	auto [ec, n] = co_await async_connect(m_socket, endpoints, boost::asio::experimental::as_tuple(boost::asio::use_awaitable));

	if (ec)
	{
		std::cout << ec.message() << std::endl;
	}
	else
	{
		std::cout << "Connected!" << std::endl;
	}
}

awaitable<std::string> SimpleClient::Read() {	
	std::cout << "reading data" << std::endl;
	std::vector<char> response(1024);
	auto [ec, n] = co_await m_socket.async_read_some(boost::asio::buffer(response),
		boost::asio::experimental::as_tuple(boost::asio::use_awaitable));
	if (!ec)
	{
		// TODO: For example here you could have a function that checks 
		// if the response has an id 
		//		-> handle the response to the request
		// if the response has no id
		//		-> probably a signal
		std::cout << "Read " << n << " bytes\n";
		auto res = std::string{ response.begin(), response.end() };
		std::cout << res;
		co_return res;
	}
	else
	{
		// this should be written in a better way but this handles server disconnects/reconnects automagically
		std::cout << ec.value() << std::endl;
		co_await _Connect();
	}
}

awaitable<void> SimpleClient::Write(const std::vector<char>& data) {
	std::cout << "writing data" << std::endl;
	auto [ec, n] = co_await async_write(m_socket, buffer(data), boost::asio::experimental::as_tuple(boost::asio::use_awaitable));
	std::cout << "Written " << n << " bytes\n";
	if (ec)
	{
		// this should be written in a better way but this handles server disconnects/reconnects automagically
		std::cout << ec.value() << std::endl;
		co_await _Connect();
	}
}