cmake_minimum_required(VERSION 3.22.1)
project(linkchat_brpc)

find_package(Protobuf REQUIRED)

message(STATUS "Protobuf libraries: ${Protobuf_LIBRARIES}")
message(STATUS "Protobuf include: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf executable: ${Protobuf_PROTOC_EXECUTABLE}")

set(PROTO_FILE ${CMAKE_SOURCE_DIR}/echo.proto)
set(PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(PROTO_SRC ${PROTO_GEN_DIR}/echo.pb.cc)
set(PROTO_HDR ${PROTO_GEN_DIR}/echo.pb.h)

add_custom_command(
        OUTPUT ${PROTO_SRC} ${PROTO_HDR}
        COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        ARGS --cpp_out=${PROTO_GEN_DIR}
        -I ${CMAKE_SOURCE_DIR}
        ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generating protobuf sources for echo.proto"
        VERBATIM
)

add_executable(
        rpc_test rpc_test.cpp ${PROTO_SRC}
)
target_include_directories(
        rpc_test PRIVATE
        ${Protobuf_INCLUDE_DIRS}
        ${PROTO_GEN_DIR}
)
target_link_libraries(
        rpc_test PRIVATE
        brpc leveldb gflags ssl crypto gtest fmt spdlog
        ${Protobuf_LIBRARIES}
)