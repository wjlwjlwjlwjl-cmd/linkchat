cmake_minimum_required(VERSION 3.22.1)
project(linkchat_common)

file(GLOB_RECURSE HDRS *.h)

add_library(linkchat_common INTERFACE)

target_include_directories(linkchat_common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})