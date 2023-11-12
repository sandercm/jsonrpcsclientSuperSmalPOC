#pragma once
#include "SimpleClient.h"
#include <iostream>

SimpleClient::SimpleClient(tcp::socket& socket, const std::string& host, const std::string& port)
	: m_socket(socket)
	, m_host(host)
	, m_port(port)
	, m_parser()
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

void handleMessage(boost::json::value msg)
{
	if (msg.is_object())
	{
		auto obj = msg.get_object();
		if (obj.contains("id"))
		{
			std::cout << "response contains id" << obj.at("id") << std::endl;
		}
		else
		{
			std::cout << "object has no id, so probably signal" << std::endl;
		}
	}
	else
	{
		std::cout << "recieved unknown json object " << msg << std::endl;
	}
}

awaitable<boost::json::value> SimpleClient::Read() {	
	std::cout << "reading data" << std::endl;
	std::vector<char> response(9);
	auto [ec, n] = co_await m_socket.async_read_some(boost::asio::buffer(response),
		boost::asio::experimental::as_tuple(boost::asio::use_awaitable));
	if (!ec)
	{
		std::cout << "Read " << n << " bytes\n";
		auto res = std::string{ response.begin(), response.end() }; // this part can probably be optimised
		std::cout << res << std::endl;

		// Add to parser buffer
		auto amountParsed = m_parser.write_some(res);
		std::cout << "parsed " << amountParsed << " chars" << std::endl;
		std::cout << "there are " << res.size() - amountParsed << " bytes left in the buffer" << std::endl;
		// if parser isn't done we can just not return anything to suspend the execution of read()
		// it will be called again in the while loop inside co_spawn
		if (m_parser.done())
		{
			std::cout << "m_parser is done" << std::endl;
			auto jv = m_parser.release();
			std::cout << jv << std::endl;
			m_parser.reset();
			auto subs = res.substr(amountParsed);
			std::cout << "trying to continue with this sub string " << subs << std::endl;
			m_parser.write_some(subs);
			co_return jv;
		}
	}
	else
	{
		// this should be written in a better way but this handles server disconnects/reconnects automagically
		std::cout << ec.value() << std::endl;
		co_await _Connect();
	}
}
// Will probably be replace with a co_spawn call for each method call
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