#include "SimpleClient.h"
#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

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

std::vector<std::string> splitString(const std::string& input) {
	std::vector<std::string> result;

	size_t startPos = 0;
	size_t foundPos = input.find("}{", startPos);

	while (foundPos != std::string::npos) {
		result.push_back(input.substr(startPos, foundPos - startPos + 1));
		startPos = foundPos + 2;
		foundPos = input.find("}{", startPos);
	}

	// Add the last part of the string (after the last "}{")
	result.push_back(input.substr(startPos));

	return result;
}

void handleMessage(std::string msg)
{
	auto messages = splitString(msg);

	for (const auto& json : messages)
	{
		std::cout << "json is" << json << std::endl;
		Document d;
		d.Parse(json.c_str());

		if (d.IsObject() && d.HasMember("id"))
		{
			std::cout << d["id"].GetInt() << std::endl;
			std::cout << json << std::endl;
		}
		else
		{
			std::cout << "Message recieved with no id" << std::endl;
			std::cout << json << std::endl;
		}
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
		handleMessage(res);
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