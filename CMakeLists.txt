cmake_minimum_required(VERSION 3.15)
project(sanntid)
set(CMAKE_CXX_STANDARD 20)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(SDL2 CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)

add_executable(sanntid main.cpp)

add_subdirectory(Tests)

target_link_libraries(sanntid PRIVATE
        Boost::system
        SDL2::SDL2 SDL2::SDL2main
        opencv_core opencv_highgui opencv_imgproc opencv_imgcodecs opencv_videoio
        nlohmann_json::nlohmann_json
)

if (MINGW)
    target_link_libraries(boost_networking PRIVATE ws2_32)
endif ()

