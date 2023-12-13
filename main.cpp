#define CVUI_IMPLEMENTATION
#include "cvui.h"
#include "FrameReceiver.hpp"
#include "CommandSender.hpp"
#include "RobotController.hpp"
#include <boost/asio.hpp>


#define WINDOW_NAME "Sphero control & Camera Feed" // Name of the window

int main() {
    boost::asio::io_service io_service;
    FrameReceiver frameReceiver(io_service, "10.25.45.112", "8000");
    CommandSender commandSender(io_service, "10.25.45.112", "8001");
    RobotController robotController("10.25.45.112", "8000", "10.25.45.112", "8001", io_service, commandSender);

    float speed = 0;
    bool manualMode = true;

    frameReceiver.startReceiving();
    robotController.start();
    cvui::init(WINDOW_NAME);
    cv::Mat frame = cv::Mat(650, 1000, CV_8UC3);
    cv::Mat videoFrame;

    while (true) {

        frame = cv::Scalar(49, 52, 49);  // Clear the frame

        frameReceiver.getNextFrame(videoFrame);

        if (not videoFrame.empty()) {
            cvui::image(frame, 50, 50, videoFrame);
            cvui::text(frame, 50, 10, "Camera:", 0.8); // Video feed
        }

        cvui::trackbar(frame, 50, 540, 360, &speed, (float)0.5, (float)1.125);

        int key = cv::waitKey(20); // Declare 'key' here so it's available throughout the loop

        // Window displaying driving mode selection
        cvui::window(frame, 50, 400, 180, 120, "Select driving mode");

        if (cvui::button(frame, 80, 440, "Manual")) {
            manualMode = true;
        }
        if (cvui::button(frame, 80, 480, "Automatic")) {
            manualMode = false;
        }

        if (manualMode) {
            // text displaying when in manual mode
            cvui::text(frame, 300, 420, "Manual Control Active:");
            nlohmann::json j;
            if (key == 119) {  // 'w' key for Forward
                j["command"] = "F";
                j["speed"] = speed;
            } else if (key == 97) {  // 'a' key for Left
                j["command"] = "FL";
                j["speed"] = speed;
            } else if (key == 100) {  // 'd' key for Right
                j["command"] = "FR";
                j["speed"] = speed;
            } else if (key == 115) {  // 's' key for Backward
                j["command"] = "B";
                j["speed"] = speed;
            } else if (key == 32) {  // Space bar for Stop
                j["command"] = "S";
            }

            if (!j.empty()) {
                std::string jsonString = j.dump();
                std::cout << jsonString << std::endl;
                commandSender.sendCommand(jsonString);
            }

        } else {
            if (!videoFrame.empty()) {
                robotController.processFrame(videoFrame);
                robotController.setSpeed(speed);
            }
            // text displaying when in automatic mode
            cvui::text(frame, 300, 420, "Automatic Control Active");
        }

        //cvui::checkbox(frame, 50, 250, "Autonomous Mode", &manualMode);

        cvui::text(frame, 550, 10, "Sensor Data from Sphero RVR:", 0.8); // Data from Sphero

        cvui::text(frame, 280, 400, "Current Active Mode:", 0.6); // Active drive mode

        // Display manual control buttons
        cvui::text(frame, 50, 260, "Manual Control:");
        cvui::text(frame, 50, 280, "W - Forward");
        cvui::text(frame, 50, 300, "A - Left");
        cvui::text(frame, 50, 320, "S - Backward");
        cvui::text(frame, 50, 340, "D - Right");
        cvui::text(frame, 50, 360, "Space - Stop");
        cvui::text(frame, 350, 360, "ESC - Quit");

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
