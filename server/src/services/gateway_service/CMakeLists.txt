cmake_minimum_required(VERSION 3.22.1)
project(linkchat-gateway)

add_executable(linkchat-gateway src/gateway.cc)

target_link_libraries(
    linkchat-gateway PUBLIC
    adapters linkchat_proto
)
