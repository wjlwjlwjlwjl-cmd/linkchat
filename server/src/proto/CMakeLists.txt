cmake_minimum_required(VERSION 3.22.1)
project(linkchat_proto)

find_package(Protobuf REQUIRED)

# 模块化构建，将 proto 代码构建为静态库
# base.proto
protobuf_generate_cpp(BASE_PROTO_SRC BASE_PROTO_HDR base.proto)
add_library(base_proto STATIC ${BASE_PROTO_SRC} ${BASE_PROTO_HDR})
target_include_directories(
    base_proto PUBLIC 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(base_proto PUBLIC protobuf)

# file.proto
protobuf_generate_cpp(FILE_PROTO_SRC FILE_PROTO_HDR file.proto)
add_library(file_proto STATIC ${FILE_PROTO_SRC} ${FILE_PROTO_HDR})
target_include_directories(file_proto PUBLIC 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
    file_proto PUBLIC 
    protobuf 
    base_proto
)

# message.proto
protobuf_generate_cpp(MESSAGE_PROTO_SRC MESSAGE_PROTO_HDR message.proto)
add_library(
    message_proto ${MESSAGE_PROTO_SRC} ${MESSAGE_PROTO_HDR}
)
target_include_directories(
    message_proto PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
    message_proto PUBLIC
    protobuf
    base_proto
)