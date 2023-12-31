#ifndef SANNTID_COMMANDSENDER_HPP
#define SANNTID_COMMANDSENDER_HPP

#include <string>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

class CommandSender {
public:
    CommandSender(boost::asio::io_service& io_service, std::string  server, std::string  port);
    void sendCommand(const std::string& rawMessage);
    [[nodiscard]] bool isConnected() const;
    void reconnect();
    bool connectSocket();

private:
    boost::asio::io_service& io_service;
    std::string server;
    std::string port;
    boost::asio::ip::tcp::socket command_socket;
};

#endif //SANNTID_COMMANDSENDER_HPP