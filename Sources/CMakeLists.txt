cmake_minimum_required (VERSION 2.8.11)
set(CMAKE_CXX_STANDARD 20)

project(NetworkingCpp)

add_subdirectory (Common)
add_subdirectory (Client)
add_subdirectory (Server)

include_directories(${PROJECT_SOURCE_DIR}/Lib/asio-1.18.1/include)