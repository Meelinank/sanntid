#define CVUI_IMPLEMENTATION
#include "cvui.h"
#include "FrameReceiver.hpp"
#include "CommandSender.hpp"
#include "RobotController.hpp"
#include <boost/asio.hpp>

#define WINDOW_NAME "Robot Control" // Name of the window

int main() {
    boost::asio::io_service io_service;
    FrameReceiver frameReceiver(io_service, "10.25.45.112", "8000");
    CommandSender commandSender(io_service, "10.25.45.112", "8001");
    RobotController robotController("10.25.45.112", "8000", "10.25.45.112", "8001", io_service, commandSender);

    bool manualMode = true;
    frameReceiver.startReceiving();
    robotController.start();
    cvui::init(WINDOW_NAME);

    cv::Mat frame = cv::Mat(400, 600, CV_8UC3);

    while (true) {
        frame = cv::Scalar(49, 52, 49);  // Clear the frame

        cv::Mat videoFrame;
        if (frameReceiver.getNextFrame(videoFrame)) {
            cv::imshow("Video Stream", videoFrame);  // Show the video frame
        }

        int key = cv::waitKey(20); // Declare 'key' here so it's available throughout the loop

        if (manualMode) {
            if (key == 119) {  // 'w' key for Forward
                commandSender.sendCommand("F");
            } else if (key == 97) {  // 'a' key for Left
                commandSender.sendCommand("FL");
            } else if (key == 100) {  // 'd' key for Right
                commandSender.sendCommand("FR");
            } else if (key == 115) {  // 's' key for Backward
                commandSender.sendCommand("B");
            } else if (key == 32) {  // Space bar for Stop
                commandSender.sendCommand("S");
            }
        } else {
            if (!videoFrame.empty()) {
                robotController.processFrame(videoFrame);
            }
        }

        cvui::checkbox(frame, 50, 250, "Autonomous Mode", &manualMode);

        cvui::update();
        cv::imshow(WINDOW_NAME, frame);

        if (key == 27) { // 'ESC' key to break
            break;
        }
    }

    robotController.stop();
    frameReceiver.stopReceiving();

    return 0;
}