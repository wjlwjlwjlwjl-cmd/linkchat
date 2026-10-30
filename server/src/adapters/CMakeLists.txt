cmake_minimum_required(VERSION 3.22.1)
project(linkchat_adapter)

add_library(adapters INTERFACE)

target_include_directories(
    adapters INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    adapters INTERFACE 
    alibabacloud-oss-cpp-sdk
    brpc leveldb gflags ssl crypto gtest fmt spdlog cpprest etcd-cpp-api
    amqpcpp ev pthread hiredis redis++ odb odb-boost odb-mysql jsoncpp
    cpr elasticlient curl
)