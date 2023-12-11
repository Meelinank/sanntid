#ifndef SANNTID_ROBOTCONTROLLER_HPP
#define SANNTID_ROBOTCONTROLLER_HPP

#include "FrameReceiver.hpp"
#include "CommandSender.hpp"
#include <opencv2/opencv.hpp>
#include <atomic>
#include <thread>

class RobotController {
public:
    RobotController(const std::string& videoServer, const std::string& videoPort,
                    const std::string& commandServer, const std::string& commandPort,
                    boost::asio::io_service& io_service, CommandSender& cmdSender);
    void start();
    void stop();
    void processFrame(const cv::Mat& frame);

private:
    FrameReceiver frameReceiver;
    CommandSender& commandSender;
    std::atomic<bool> running;
    std::thread processingThread;
    int lastKnownHeading;
    void frameProcessingLoop();
};

#endif //SANNTID_ROBOTCONTROLLER_HPP
