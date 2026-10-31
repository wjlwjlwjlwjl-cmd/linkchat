cmake_minimum_required(VERSION 3.22.1)
project(linkchat-server)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(proto)
add_subdirectory(adapters)
add_subdirectory(util)
add_subdirectory(constants)

add_subdirectory(services/file_service)
add_subdirectory(services/message_service)