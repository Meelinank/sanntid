find_package(Catch2 3 REQUIRED)
add_executable(testConnection testConnection.cpp)
target_link_libraries(testConnection PRIVATE Catch2::Catch2WithMain)