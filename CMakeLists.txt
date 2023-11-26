cmake_minimum_required(VERSION 3.15)
project(sanntid)
set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS system)
find_package(SDL2 CONFIG REQUIRED)
find_package(OpenCV REQUIRED)
add_subdirectory(UI)

add_executable(sanntid main.cpp)
add_executable(UI_test UI/UI_test.cpp)
include_directories(${OpenCV_INCLUDE_DIRS})

target_link_libraries(UI_test ${OpenCV_LIBS})

add_subdirectory(Tests)

target_link_libraries(sanntid PUBLIC
        Boost::system
        SDL2::SDL2 SDL2::SDL2main
        opencv_core opencv_highgui opencv_imgproc opencv_imgcodecs opencv_videoio
)

#if (MINGW)
 #   target_link_libraries(boost_networking PRIVATE ws2_32)
#endif ()

