add_executable(NetClient "Client.cpp")

target_link_libraries(NetClient LINK_PUBLIC Common)
include_directories("../../Lib/asio-1.18.1/include")