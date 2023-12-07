#ifndef SANNTID_ROBOTCONTROLLER_HPP
#define SANNTID_ROBOTCONTROLLER_HPP

#include "FrameReceiver.hpp"
#include "CommandSender.hpp"
#include <opencv2/opencv.hpp>

class RobotController {
public:
    RobotController(const std::string& videoServer, const std::string& videoPort,
                    const std::string& commandServer, const std::string& commandPort,
                    boost::asio::io_service& io_service);
    void start();
    void stop();
    void processFrame(const cv::Mat& frame);

private:
    FrameReceiver frameReceiver;
    CommandSender commandSender;
    std::string lastKnownDirection;
    std::atomic<bool> running;
    std::thread processingThread;
    void frameProcessingLoop();
};

#endif //SANNTID_ROBOTCONTROLLER_HPP