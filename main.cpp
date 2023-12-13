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

    float speed = 0;
    bool manualMode = true;
    int keyTimer = 0;

    frameReceiver.startReceiving();
    robotController.start();
    cvui::init(WINDOW_NAME);
    cv::Mat frame = cv::Mat(650, 1000, CV_8UC3);
    cv::Mat videoFrame;

    // Variable to store the last received sensor data
    nlohmann::json lastSensorData;

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
            nlohmann::json j;
            j["command"] = "MANUAL";
            // Include any other relevant data
            std::string jsonString = j.dump();
            commandSender.sendCommand(jsonString);
        }

        if (cvui::button(frame, 80, 480, "Automatic")) {
            manualMode = false;
        }

        // Sensor Data Display Section
        cvui::window(frame, 450, 10, 300, 200, "Sensor Data from Sphero RVR");
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
        int yPos = 40; // Starting Y position for displaying data
        for (auto& [key, value] : lastSensorData.items()) {
            std::string text = key + ": " + value.dump();
            cvui::text(frame, 460, yPos, text, 0.4);
            yPos += 20; // Increment Y position for next item
        }

        if (manualMode) {
            keyTimer++;
            cvui::text(frame, 300, 420, "Manual Control Active", 0.6);
            nlohmann::json j;
            if (key == 119) {  // 'w' key for Forward
                j["direction"] = "F";
                j["speed"] = speed;
                keyTimer = 0;
            } else if (key == 97) {  // 'a' key for Left
                j["direction"] = "FL";
                j["speed"] = speed;
                keyTimer = 0;
            } else if (key == 100) {  // 'd' key for Right
                j["direction"] = "FR";
                j["speed"] = speed;
                keyTimer = 0;
            } else if (key == 115) {  // 's' key for Backward
                j["direction"] = "B";
                j["speed"] = speed;
                keyTimer = 0;
            }  else if (keyTimer > 10) {  // Space bar for Stop
                j["direction"] = "S";
            }

            if (!j.empty()) {
                std::string jsonString = j.dump();
                std::cout << jsonString << std::endl;
                commandSender.sendCommand(jsonString);
            }

        } else {
            keyTimer = 0;
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
