cmake_minimum_required(VERSION 3.15)
project(sanntid)
set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenCV REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(sanntid main.cpp src/FrameReceiver.cpp src/RobotController.cpp src/CommandSender.cpp src/SensorDataReceiver.cpp)

include_directories(${OpenCV_INCLUDE_DIRS})

target_include_directories(sanntid PUBLIC "${CMAKE_SOURCE_DIR}/Include")
target_include_directories(sanntid PUBLIC "${CMAKE_SOURCE_DIR}/UI")

add_subdirectory(Tests)
add_subdirectory(src)


target_link_libraries(sanntid PUBLIC
        Boost::system
        opencv_core opencv_highgui opencv_imgproc opencv_imgcodecs opencv_videoio
        nlohmann_json::nlohmann_json
)

#if (MINGW)
 #   target_link_libraries(boost_networking PRIVATE ws2_32)
#endif ()

