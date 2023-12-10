#include "RobotController.hpp"
#include <iostream>
#include <chrono>

RobotController::RobotController(const std::string &videoServer, const std::string &videoPort,
                                 const std::string &commandServer, const std::string &commandPort,
                                 boost::asio::io_service &io_service, CommandSender &cmdSender)
        : frameReceiver(io_service, videoServer, videoPort),
          commandSender(cmdSender),
          running(false) {
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
        cv::inRange(hsvFrame, cv::Scalar(35, 50, 50), cv::Scalar(75, 255, 255), greenMask);

        cv::erode(greenMask, greenMask, cv::Mat(), cv::Point(-1, -1), 2);
        cv::dilate(greenMask, greenMask, cv::Mat(), cv::Point(-1, -1), 2);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(greenMask, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

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
            int cy = static_cast<int>(m.m01 / m.m00);

            // Example of decision logic based on the object's position
            if (cx < frame.cols / 3) {
                lastKnownDirection = "L"; // Turn left
            } else if (cx > 2 * frame.cols / 3) {
                lastKnownDirection = "R"; // Turn right
            } else {
                lastKnownDirection = "F"; // Move forward
            }
        } else {
            lastKnownDirection = "S"; // Stop if the object is not found
        }

        std::cout << lastKnownDirection << std::endl;

        commandSender.sendCommand(lastKnownDirection);
    }
    catch(std::exception & e)
    {
        std::cerr << "Error in frame processing: " << e.what() << std::endl;
    }
}
