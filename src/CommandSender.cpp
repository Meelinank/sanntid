#include "CommandSender.hpp"
#include <iostream>
#include <chrono>

CommandSender::CommandSender(boost::asio::io_service& io_service, const std::string& server, const std::string& port)
        : command_socket(io_service) {
    // Resolve the server address and port
    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query(server, port);
    boost::asio::connect(command_socket, resolver.resolve(query));
}

void CommandSender::sendCommand(const std::string& command) {
    std::this_thread::sleep_for(std::chrono::milliseconds(33));
    try {
        nlohmann::json j;
        j["command"] = command;
        std::string message = j.dump();

        boost::asio::write(command_socket, boost::asio::buffer(message));
    } catch (std::exception& e) {
        std::cerr << "Failed to send command: " << e.what() << std::endl;
        reconnect();
    }
}

void CommandSender::reconnect() {
    if (command_socket.is_open()) {
        command_socket.close();
        std::cout << "Reconnecting to command server" << std::endl;
        command_socket.connect(command_socket.remote_endpoint());
        std::cout << "Reconnect successful" << std::endl;
    }
}
