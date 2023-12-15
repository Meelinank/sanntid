#define CVUI_IMPLEMENTATION
#include "cvui.h"
#include "FrameReceiver.hpp"
#include "CommandSender.hpp"
#include "SensorDataReceiver.hpp"
#include "RobotController.hpp"
#include <boost/asio.hpp>

#define WINDOW_NAME "Sphero control & Camera Feed"

// Function to convert RGB to Unsigned Scalar Long Integer, this function is generated by ChatGPT
unsigned int RGBtoUSLI(const cv::Scalar& color) {
    return ((unsigned int)color[2] & 0xff) << 16 | ((unsigned int)color[1] & 0xff) << 8 | ((unsigned int)color[0] & 0xff); // Convert RGB to USLI
}

int main() {
    boost::asio::io_service io_service;
    FrameReceiver frameReceiver(io_service, "10.25.45.112", "8000");
    CommandSender commandSender(io_service, "10.25.45.112", "8001");
    SensorDataReceiver sensorDataReceiver(io_service, "10.25.45.112", "8002");
    RobotController robotController("10.25.45.112", "8000", "10.25.45.112", "8001", io_service, commandSender);
    commandSender.connectSocket();

    // Declaring UI variables
    float speed = 0;
    bool manualMode = true;
    int keyTimer = 0;

    frameReceiver.startReceiving();
    robotController.start();
    cvui::init(WINDOW_NAME);
    cv::Mat frame = cv::Mat(650, 840, CV_8UC3);
    cv::Mat videoFrame;

    nlohmann::json lastSensorData;

    while (true) {
        frame = cv::Scalar(10, 10, 10);

        frameReceiver.getNextFrame(videoFrame);

        if (not videoFrame.empty()) {
            cvui::image(frame, 50, 50, videoFrame);
            cvui::text(frame, 50, 10, "Camera Feed:", 0.8);
        }
        cvui::text(frame, 400, 400, "Adjust speed in manual mode, 0 is none 1 is max speed:", 0.4);
        cvui::trackbar(frame, 400, 420, 360, &speed, (float)0, (float)1);

        int key = cv::waitKey(20); // Check for key presses

        cvui::window(frame, 50, 500, 180, 120, "Select driving mode"); // Window for driving mode selection

        if (cvui::button(frame, 80, 540, "Manual")) {
            manualMode = true;
            nlohmann::json j;
            j["command"] = "MANUAL";

            std::string jsonString = j.dump();
            commandSender.sendCommand(jsonString);
        }
        if (cvui::button(frame, 80, 580, "Automatic")) {
            manualMode = false;
        }
        cvui::text(frame, 440, 10, "Sensor Data:", 0.8); // Data from Sphero
        cvui::window(frame, 440, 50, 350, 250, "Sensor Data from Sphero RVR");
        if (sensorDataReceiver.isConnected()) {
            try {
                nlohmann::json sensorData = sensorDataReceiver.receiveSensorData();
                if (!sensorData.empty()) {
                    lastSensorData = sensorData; // Update the last received sensor data
                }
            } catch (std::exception& e) {
                std::cerr << "Error reading sensor data: " << e.what() << std::endl;
            }
        }

        // Display the last received (or stored) sensor data
        int yPos = 90;
        for (auto& [key, value] : lastSensorData .items()) {
            std::string text = key + ": " + value.dump();
            cvui::text(frame, 460, yPos, text, 0.4);
            yPos += 20;
        }

        std::cout << lastSensorData << std::endl;

        if (manualMode) {  // If in manual mode, check for key presses
            keyTimer++;
            cvui::text(frame, 260, 520, "Manual Control Active", 0.6, RGBtoUSLI(cv::Scalar(0, 255, 0)));
            nlohmann::json j;
            if (key == 119) {  // 'w' key for Forward
                j["command"] = "MANUAL";
                j["direction"] = "F";
                j["speed"] = speed;
                keyTimer = 0;
            } else if (key == 97) {  // 'a' key for Left
                j["command"] = "MANUAL";
                j["direction"] = "L";
                j["speed"] = speed;
                keyTimer = 0;
            } else if (key == 100) {  // 'd' key for Right
                j["command"] = "MANUAL";
                j["direction"] = "R";
                j["speed"] = speed;
                keyTimer = 0;
            } else if (key == 115) {  // 's' key for Backward
                j["command"] = "MANUAL";
                j["direction"] = "B";
                j["speed"] = speed;
                keyTimer = 0;
            }  else if (keyTimer > 10) {  // Space bar for Stop
                j["command"] = "MANUAL";
                j["direction"] = "S";
            }

            if (!j.empty()) {
                std::string jsonString = j.dump();
                std::cout << jsonString << std::endl;
                commandSender.sendCommand(jsonString);
            }

        } else {
            keyTimer = 0;
            if (!videoFrame.empty()) {
                robotController.processFrame(videoFrame);
                robotController.setSpeed(speed);
            }
            cvui::text(frame, 260, 520, "Automatic Control Active", 0.6, RGBtoUSLI(cv::Scalar(0, 0, 255)));
        }

        if (!manualMode) {
            robotController.setSpeed(speed);
            if (!videoFrame.empty()) {
                robotController.processFrame(videoFrame);
            }
        }

        cvui::text(frame, 260, 500, "Current Active Mode:", 0.6);

        cvui::text(frame, 535, 130, "%" );

        // Display manual control buttons
        cvui::text(frame, 50, 320, "Manual Control:");
        cvui::text(frame, 50, 340, "W - Forward");
        cvui::text(frame, 50, 360, "A - Left");
        cvui::text(frame, 50, 380, "S - Backward");
        cvui::text(frame, 50, 400, "D - Right");
        cvui::text(frame, 50, 420, "Space - Stop");
        cvui::text(frame, 660, 600, "ESC - Quit", 0.6, RGBtoUSLI(cv::Scalar(0, 0, 255)));

        cvui::update();
        cv::imshow(WINDOW_NAME, frame);

        if (key == 27) { // 'ESC' key to exit
            break;
        }
    }
    // Stop servers
    io_service.stop();
    cv::destroyWindow(WINDOW_NAME);
    robotController.stop();
    frameReceiver.stopReceiving();

    std::cout << "Exiting program" << std::endl;
    return 0;
}