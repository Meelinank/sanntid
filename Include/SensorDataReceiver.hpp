#ifndef SANNTID_SENSORDATARECEIVER_HPP
#define SANNTID_SENSORDATARECEIVER_HPP

#include <string>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

class SensorDataReceiver {
public:
    SensorDataReceiver(boost::asio::io_service& io_service, const std::string& server, const std::string& port);
    nlohmann::json receiveSensorData();
    bool isConnected() const;
    void reconnect();

private:
    boost::asio::io_service& io_service;
    std::string server;
    std::string port;
    boost::asio::ip::tcp::socket data_socket;
    bool connectSocket();
};

#endif //SANNTID_SENSORDATARECEIVER_HPP
