#pragma once
#include <iostream>
#define BOOST_CONTAINER_NO_LIB 
#define BOOST_JSON_NO_LIB
#include <boost/json/src.hpp>
#include "SimpleClient.h"

using namespace boost::asio;
using namespace boost::asio::ip;

int main() {
    io_context ioContext;
    tcp::socket socket(ioContext);

    std::string serverHost = "127.0.0.1";
    std::string serverPort = "9090";
    SimpleClient client(socket, serverHost, serverPort);
    // This spawns the coroutine that handles writing
    // this will do a non blocking write on the socket
    co_spawn(ioContext, [&client]() mutable -> awaitable<void> {
        co_await client.Connect();
        std::string test = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"sum\",\"params\":[1,2,3]}{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"sum\",\"params\":[1,2,3]}{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"sum\",\"params\":[1,2,3]}";
        std::vector<char> buffer{ test.begin(),test.end() };
        co_await client.Write(buffer);
    }, detached);

    // This spawns the corountine that handles reading
    // this will do a non blocking read on the socket
    co_spawn(ioContext, [&client]() mutable -> awaitable<void> {
        co_await client.Connect();
        while (true) {
            co_await client.Read();
        }
        }, detached);

    ioContext.run(); // Run the io_context to start the asynchronous operations

    return 0;
}
