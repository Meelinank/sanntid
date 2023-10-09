#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main() {
    boost::asio::io_service io_service;

    tcp::resolver resolver(io_service);
    tcp::resolver::query query("RASPBERRY_PI_IP_ADDRESS", "12345");
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    tcp::socket socket(io_service);
    boost::asio::connect(socket, endpoint_iterator);

    std::string message = "Hello from C++!";
    boost::asio::write(socket, boost::asio::buffer(message));

    return 0;
}
