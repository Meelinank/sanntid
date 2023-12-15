#include "RobotController.hpp"
#include <iostream>
#include <chrono>

RobotController::RobotController(const std::string &videoServer, const std::string &videoPort,
                                 const std::string &commandServer, const std::string &commandPort,
                                 boost::asio::io_service &io_service, CommandSender &cmdSender)
        : frameReceiver(io_service, videoServer, videoPort),
          commandSender(cmdSender),
          running(false), lastKnownHeading(0) {
}

void RobotController::start() {
    running = true;
    frameReceiver.startReceiving();
    processingThread = std::thread(&RobotController::frameProcessingLoop, this);
}

void RobotController::stop() {
    running = false;
    frameReceiver.stopReceiving();
    if (processingThread.joinable()) {
        processingThread.join();
    }
}

void RobotController::setSpeed(float newSpeed) {
    speed = newSpeed;
}

void RobotController::frameProcessingLoop() {
    while (running) {
        cv::Mat frame;
        if (frameReceiver.getNextFrame(frame)) {
            processFrame(frame);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

void RobotController::processFrame(const cv::Mat& frame) {
    try {
        cv::Mat hsvFrame;
        cv::cvtColor(frame, hsvFrame, cv::COLOR_BGR2HSV);
        cv::Mat greenMask;
        cv::inRange(hsvFrame, cv::Scalar(35, 100, 100), cv::Scalar(75, 255, 255), greenMask);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(greenMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        double largestArea = 0.0;
        int largestContourIndex = -1;

        for (size_t i = 0; i < contours.size(); i++) {
            double area = cv::contourArea(contours[i]);
            if (area > largestArea) {
                largestArea = area;
                largestContourIndex = i;
            }
        }

        if (largestContourIndex != -1) {
            cv::Moments m = cv::moments(contours[largestContourIndex]);
            int cx = static_cast<int>(m.m10 / m.m00);
            int heading = static_cast<int>(70 - 140 * (static_cast<float>(cx) / frame.cols));

            if (largestArea < 5000) { // Define AREA_THRESHOLD as per your requirement
                // Object detected but not close enough
                lastKnownHeading = heading;
                nlohmann::json j;
                j["command"] = "AUTO";
                j["heading"] = heading;
                j["speed"] = speed; // Use the speed value
                std::string jsonString = j.dump();

                std::cout << jsonString << std::endl;
                commandSender.sendCommand(jsonString);
            } else {
                // Object is close, stop the robot
                nlohmann::json j;
                j["command"] = "S"; // You might need to handle this command in your robot control logic
                std::string jsonString = j.dump();

                std::cout << jsonString << std::endl;
                commandSender.sendCommand(jsonString);
            }
        } else {
            // No object detected, or object lost
            // Here, you can decide whether to stop or continue in the last known direction
            nlohmann::json j;
            j["command"] = "AUTO"; // or "AUTO" to continue in the last known direction
            j["heading"] = lastKnownHeading;
            j["speed"] = 0.7; // Use the speed value
            std::string jsonString = j.dump();

            std::cout << jsonString << std::endl;
            commandSender.sendCommand(jsonString);
        }
    }
    catch (std::exception & e) {
        std::cerr << "Error in frame processing: " << e.what() << std::endl;
    }
}
