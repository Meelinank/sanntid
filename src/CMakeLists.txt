add_library(libs CommandSender.cpp FrameReceiver.cpp RobotController.cpp SensorDataReceiver.cpp)

target_include_directories(libs PUBLIC "${PROJECT_SOURCE_DIR}/Include")

target_link_libraries(libs PUBLIC
        Boost::system
        opencv_core opencv_highgui opencv_imgproc opencv_imgcodecs opencv_videoio
        nlohmann_json::nlohmann_json
)