add_executable (NetClient Client.cpp)

target_link_libraries (NetClient LINK_PUBLIC Common)
include_directories(${PROJECT_SOURCE_DIR}/Lib/asio-1.18.1/include)