#include "CommandSender.hpp"
#include <iostream>

CommandSender::CommandSender(boost::asio::io_service& io_service, const std::string& server, const std::string& port)
        : command_socket(io_service) {
    // Resolve the server address and port
    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query(server, port);
    boost::asio::connect(command_socket, resolver.resolve(query));
}

void CommandSender::sendCommand(const std::string& command) {
    try {
        nlohmann::json j;
        j["command"] = command;
        std::string message = j.dump();

        boost::asio::write(command_socket, boost::asio::buffer(message));
    } catch (std::exception& e) {
        std::cerr << "Failed to send command: " << e.what() << std::endl;
    }
}
