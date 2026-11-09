cmake_minimum_required(VERSION 3.22.1)
project(linkchat_proto)

find_package(Protobuf REQUIRED)

cmake_minimum_required(VERSION 3.22.1)
project(linkchat_proto)

find_package(Protobuf REQUIRED)

# 全部proto列表，不再按模块，直接全部打成一个静态包
set(PROTO_FILES
    base.proto
    file.proto
    message.proto
    user.proto
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_library(linkchat_proto STATIC ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(
    linkchat_proto PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(linkchat_proto PUBLIC protobuf)