add_executable(NetServer "Server.cpp")

target_link_libraries(NetServer LINK_PUBLIC Common)
include_directories(${PROJECT_SOURCE_DIR}/Lib/asio-1.18.1/include)