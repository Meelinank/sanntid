#ifndef SANNTID_COMMANDSENDER_HPP
#define SANNTID_COMMANDSENDER_HPP

#include <string>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

class CommandSender {
public:
    CommandSender(boost::asio::io_service& io_service, const std::string& server, const std::string& port);
    void sendCommand(const std::string& command, int heading = 0); // Modified to include heading
    bool isConnected() const;
    void reconnect();

private:
    boost::asio::io_service& io_service; // Store reference to io_service
    std::string server;
    std::string port;
    boost::asio::ip::tcp::socket command_socket;
    bool connectSocket(); // New function to handle connection
};

#endif //SANNTID_COMMANDSENDER_HPP
