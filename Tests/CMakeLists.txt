find_package(Catch2 3 REQUIRED)

include_directories("${PROJECT_SOURCE_DIR}/src")
#add_test(NAME test_RobotController COMMAND test_RobotController)

add_executable(tester tester.cpp)
target_link_libraries(tester PRIVATE libs Catch2::Catch2WithMain)
