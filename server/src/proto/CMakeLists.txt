cmake_minimum_required(VERSION 3.22.1)
project(linkchat_proto)

find_package(Protobuf REQUIRED)

# base.proto
protobuf_generate_cpp(BASE_PROTO_SRC BASE_PROTO_HDR base.proto)
add_library(base_proto STATIC ${BASE_PROTO_SRC} ${BASE_PROTO_HDR})
target_include_directories(base_proto PUBLIC 
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(base_proto PUBLIC protobuf)

# file.proto
protobuf_generate_cpp(FILE_PROTO_SRC FILE_PROTO_HDR file.proto)
add_library(file_proto STATIC ${FILE_PROTO_SRC} ${FILE_PROTO_HDR})
target_include_directories(file_proto PUBLIC 
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(file_proto PUBLIC 
    protobuf 
    base_proto
)