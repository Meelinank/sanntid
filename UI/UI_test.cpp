#define CVUI_IMPLEMENTATION
#include <opencv2/opencv.hpp>
#include "cvui.h"

#define WINDOW_NAME "Sphero Control"

int main() {
    cv::Mat frame = cv::Mat(400, 600, CV_8UC3);
    int sliderValue = 50;
    bool isManualMode = true; // Variable to track the mode
    bool isColorTrackingEnabled = false; // Variable to track color tracking state
    std::string currentLightColor = "None"; // Variable to track the light color

    cvui::init(WINDOW_NAME);

    while (true) {
        frame = cv::Scalar(49, 52, 49);

        // Mode selection
        if (cvui::button(frame, 50, 50, "Manual Steering") && !isManualMode) {
            isManualMode = true;
        }
        if (cvui::button(frame, 200, 50, "Automatic Driving") && isManualMode) {
            isManualMode = false;
        }

        // Toggle Color Tracking
        if (cvui::button(frame, 350, 50, isColorTrackingEnabled ? "Disable Color Tracking" : "Enable Color Tracking")) {
            isColorTrackingEnabled = !isColorTrackingEnabled;
        }

        // Display current mode
        cvui::printf(frame, 50, 20, 0.4, 0xFFFFFF, "Current Mode: %s", isManualMode ? "Manual" : "Automatic");

        // Light color selection
        if (cvui::button(frame, 50, 150, "Red Light")) {
            currentLightColor = "Red";
            // Code to change the light to red
        }
        if (cvui::button(frame, 150, 150, "Green Light")) {
            currentLightColor = "Green";
            // Code to change the light to green
        }
        if (cvui::button(frame, 250, 150, "Blue Light")) {
            currentLightColor = "Blue";
            // Code to change the light to blue
        }

        // Display current light color
        cvui::printf(frame, 50, 130, 0.4, 0xFFFFFF, "Light Color: %s", currentLightColor.c_str());

        // UI components for manual mode
        if (isManualMode) {
            cvui::trackbar(frame, 50, 100, 300, &sliderValue, 0, 100);
        }
        // Add UI components for automatic mode if needed

        // Add color tracking code here if isColorTrackingEnabled is true

        // Update the window content
        cvui::update();

        // Show everything on the screen
        cv::imshow(WINDOW_NAME, frame);

        // Check for keyboard interactions
        if (cv::waitKey(20) == 27) {
            break;
        }
    }

    return 0;
}
