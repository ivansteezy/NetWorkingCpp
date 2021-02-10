add_library(Common "Net.hpp" "Net.cpp" "NetClient.hpp" "NetCommon.hpp" "NetConnection.hpp" "NetMessage.hpp" "NetServer.hpp" "NetThreadSafeQueue.hpp")

include_directories(${PROJECT_SOURCE_DIR}/Lib/asio-1.18.1/include)
target_include_directories(Common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})