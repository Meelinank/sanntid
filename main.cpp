#define CVUI_IMPLEMENTATION
#include "cvui.h"
#include "FrameReceiver.hpp"
#include "CommandSender.hpp"
#include "SensorDataReceiver.hpp"
#include "RobotController.hpp"
#include <boost/asio.hpp>

#define WINDOW_NAME "Sphero control & Camera Feed"

int main() {
    boost::asio::io_service io_service;
    FrameReceiver frameReceiver(io_service, "10.25.45.112", "8000");
    CommandSender commandSender(io_service, "10.25.45.112", "8001");
    SensorDataReceiver sensorDataReceiver(io_service, "10.25.45.112", "8002");
    RobotController robotController("10.25.45.112", "8000", "10.25.45.112", "8001", io_service, commandSender);

    sensorDataReceiver.receiveSensorData();
    std::cout << "Sensor data received: " << sensorDataReceiver.receiveSensorData() << std::endl;

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

        if (!videoFrame.empty()) {
            cvui::image(frame, 50, 50, videoFrame);
            cvui::text(frame, 50, 10, "Camera Feed", 0.8);
        }

        cvui::trackbar(frame, 50, 540, 360, &speed, (float)0.5, (float)1.125);

        int key = cv::waitKey(20); // Check for key presses

        cvui::window(frame, 50, 400, 180, 120, "Driving Mode");
        if (cvui::button(frame, 80, 440, "Manual")) {
            manualMode = true;
        }
        if (cvui::button(frame, 80, 480, "Automatic")) {
            manualMode = false;
        }

        // Sensor Data Display Section
        cvui::window(frame, 450, 10, 300, 200, "Sensor Data from Sphero RVR");
        if (sensorDataReceiver.isConnected()) {
            try {
                nlohmann::json sensorData = sensorDataReceiver.receiveSensorData();
                std::cout << "Sensor data received: " << sensorData.dump() << std::endl;

                int yPos = 40; // Starting Y position for displaying data
                for (auto& [key, value] : sensorData.items()) {
                    std::string text = key + ": " + value.dump();
                    cvui::text(frame, 460, yPos, text, 0.4);
                    yPos += 20; // Increment Y position for next item
                }
            } catch (std::exception& e) {
                std::cerr << "Error reading sensor data: " << e.what() << std::endl;
                cvui::text(frame, 460, 40, "Error reading sensor data.", 0.4);
            }
        } else {
            cvui::text(frame, 460, 40, "Sensor data not connected.", 0.4);
        }

        if (manualMode) {
            cvui::text(frame, 300, 420, "Manual Control Active", 0.6);
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
            cvui::text(frame, 300, 420, "Automatic Control Active", 0.6);
            if (!videoFrame.empty()) {
                robotController.processFrame(videoFrame);
                robotController.setSpeed(speed);
            }
        }

        cvui::update();
        cv::imshow(WINDOW_NAME, frame);

        if (key == 27) { // 'ESC' key to exit
            break;
        }
    }

    robotController.stop();
    frameReceiver.stopReceiving();

    return 0;
}

