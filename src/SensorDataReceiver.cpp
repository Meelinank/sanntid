#include "SensorDataReceiver.hpp"
#include <iostream>
#include <boost/asio.hpp>
#include <utility>

SensorDataReceiver::SensorDataReceiver(boost::asio::io_service& io_service, std::string  server, std::string  port)
        : io_service(io_service), server(std::move(server)), port(std::move(port)), data_socket(io_service) {
    connectSocket();
}

bool SensorDataReceiver::connectSocket() {
    try {
        boost::asio::ip::tcp::resolver resolver(io_service);
        boost::asio::ip::tcp::resolver::query query(server, port);
        boost::asio::connect(data_socket, resolver.resolve(query));
        return true;
    } catch (std::exception& e) {
        std::cerr << "Failed to connect data socket: " << e.what() << std::endl;
        return false;
    }
}

nlohmann::json SensorDataReceiver::receiveSensorData() {
    boost::asio::streambuf buffer;
    boost::asio::read_until(data_socket, buffer, "\n");
    std::istream stream(&buffer);
    std::string data;
    std::getline(stream, data);
    try {
        return nlohmann::json::parse(data);
    } catch (std::exception& e) {
        std::cerr << "Failed to parse sensor data: " << e.what() << std::endl;
        return {};
    }
}

bool SensorDataReceiver::isConnected() const {
    return data_socket.is_open();
}
