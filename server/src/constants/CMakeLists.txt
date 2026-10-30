cmake_minimum_required(VERSION 3.22.1)
project(linkchat_util)

add_library(constants INTERFACE)

target_include_directories(constants INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})