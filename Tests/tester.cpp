#include "catch.hpp"
#include "CommandSender.hpp"
#include "RobotController.hpp"
#include "SensorDataReceiver.hpp"



TEST_CASE("Testing CommandSender", "[Testing]") { // This test works
    boost::asio::io_service io_service;
    CommandSender commandSender(io_service, "127.0.0.1", "5001");
}

TEST_CASE("Testing RobotController", "[Testing]") { // This test does not work, due to RobotController failing to connect
    boost::asio::io_service io_service;
    CommandSender commandSender(io_service, "127.0.0.1", "5001");
    RobotController robotController("127.0.0.1", "5000", "127.0.0.1", "5001", io_service, commandSender);
}

TEST_CASE("Testing FrameReceiver", "[Testing]") { // This test does not work, due to FrameReceiver failing to connect
    boost::asio::io_service io_service;
    FrameReceiver frameReceiver(io_service, "127.0.0.1", "5000");
}

TEST_CASE("Testing SensorDataReceiver", "[Testing]") { // This test works
    boost::asio::io_service io_service;
    SensorDataReceiver sensorDataReceiver(io_service, "127.0.0.1", "5002");
}