#include <iostream>
#include "SimpleClient.h"

using namespace boost::asio;
using namespace boost::asio::ip;

int main() {
    io_context ioContext;
    tcp::socket socket(ioContext);

    std::string serverHost = "127.0.0.1";
    std::string serverPort = "9090";
    SimpleClient client(socket, serverHost, serverPort);
    client.Connect();

    // This spawns the coroutine that handles writing
    // this will do a non blocking write on the socket
    co_spawn(ioContext, [&client]() mutable -> awaitable<void> {
        co_await client.Connect();
        std::string test = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"sum\",\"params\":[1,2,3]}";
        std::vector<char> buffer{ test.begin(),test.end() };
        long long i = 0;
        while (true) {
            i++;
            if (i == 1000000000)
            {
                i = 0;
                co_await client.Write(buffer);
            }
        }
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
