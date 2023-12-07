#define CVUI_IMPLEMENTATION
#include "cvui.h"
#include "FrameReceiver.hpp"
#include "CommandSender.hpp"
#include "RobotController.hpp"
#include <boost/asio.hpp>

#define WINDOW_NAME "Robot Control"

int main() {
    boost::asio::io_service io_service;
    FrameReceiver frameReceiver(io_service, "10.25.45.112", "8000");
    CommandSender commandSender(io_service, "10.25.45.112", "8001");
    RobotController robotController("10.25.45.112", "8000", "10.25.45.112", "8001", io_service);

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

        if (manualMode) {
            if (cvui::button(frame, 50, 50, "Forward")) {
                commandSender.sendCommand("F");
            }
            if (cvui::button(frame, 50, 100, "Left")) {
                commandSender.sendCommand("FL");
            }
            if (cvui::button(frame, 150, 100, "Right")) {
                commandSender.sendCommand("FR");
            }
            if (cvui::button(frame, 50, 150, "Backward")) {
                commandSender.sendCommand("B");
            }
            if (cvui::button(frame, 50, 200, "Stop")) {
                commandSender.sendCommand("S");
            }
        } else {
            if (frameReceiver.hasFrames()) {
                robotController.processFrame(videoFrame);
            }
        }

        cvui::checkbox(frame, 50, 250, "Autonomous Mode", &manualMode);

        cvui::update();
        cv::imshow(WINDOW_NAME, frame);

        if (cv::waitKey(20) == 27) {
            break;
        }
    }

    robotController.stop();
    frameReceiver.stopReceiving();

    return 0;
}