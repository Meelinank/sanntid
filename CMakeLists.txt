cmake_minimum_required(VERSION 3.15)
project(sanntid)
set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system)

add_executable(sanntid main.cpp)

target_link_libraries(sanntid PRIVATE Boost::system)