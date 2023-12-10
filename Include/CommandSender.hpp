#ifndef SANNTID_COMMANDSENDER_HPP
#define SANNTID_COMMANDSENDER_HPP

#include <string>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

class CommandSender {
public:
    CommandSender(boost::asio::io_service& io_service, const std::string& server, const std::string& port);
    void sendCommand(const std::string& command);
    void reconnect();
private:
    boost::asio::ip::tcp::socket command_socket;
};

#endif //SANNTID_COMMANDSENDER_HPP
