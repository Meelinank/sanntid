#include "CommandSender.hpp"
#include <iostream>

CommandSender::CommandSender(boost::asio::io_service& io_service, const std::string& server, const std::string& port)
        : io_service(io_service), server(server), port(port), command_socket(io_service) {
    //connectSocket();
}

bool CommandSender::connectSocket() {
    try {
        boost::asio::ip::tcp::resolver resolver(io_service);
        boost::asio::ip::tcp::resolver::query query(server, port);
        boost::asio::connect(command_socket, resolver.resolve(query));
        return true;
    } catch (std::exception& e) {
        std::cerr << "Failed to connect command socket: " << e.what() << std::endl;
        return false;
    }
}

void CommandSender::sendCommand(const std::string& rawMessage) {
    if (!isConnected()) {
        std::cerr << "Socket not connected, attempting to reconnect..." << std::endl;
        if (!connectSocket()) {
            reconnect();
            std::cerr << "Reconnect failed." << std::endl;
            return;
        }
    }

    try {
        boost::asio::write(command_socket, boost::asio::buffer(rawMessage));
    } catch (std::exception& e) {
        std::cerr << "Failed to send command: " << e.what() << std::endl;
        command_socket.close();
        connectSocket(); // Attempt to reconnect the socket
    }
}

bool CommandSender::isConnected() const {
    return command_socket.is_open();
}

void CommandSender::reconnect() {
    if (command_socket.is_open()) {
        command_socket.close();
    }
    connectSocket();
}