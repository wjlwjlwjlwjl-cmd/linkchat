cmake_minimum_required(VERSION 3.22.1)
project(linkchat_util)

add_library(util INTERFACE)

target_include_directories(
    util INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    util INTERFACE 
)