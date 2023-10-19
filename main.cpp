#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main() {
    boost::asio::io_service io_service;

    try {
        tcp::resolver resolver(io_service);
        tcp::resolver::query query("192.168.199.212", "12345");
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        tcp::socket socket(io_service);
        boost::asio::connect(socket, endpoint_iterator);

        std::string message = "Hello from C++!";
        boost::asio::write(socket, boost::asio::buffer(message));
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
