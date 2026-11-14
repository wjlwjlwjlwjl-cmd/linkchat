cmake_minimum_required(VERSION 3.22.1)
project(linkchat-server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(proto)
add_subdirectory(adapters)
add_subdirectory(util)
add_subdirectory(constants)
add_subdirectory(entity)

add_subdirectory(services/file_service)
add_subdirectory(services/message_service)
add_subdirectory(services/user_service)
add_subdirectory(services/gateway_service)
add_subdirectory(services/transmite_service)
add_subdirectory(services/friend_service)