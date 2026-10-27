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
        VERBATIM
)

set(TARGETS rpc_test etcd_registry_test etcd_discovery_test mq_test)

foreach(TARGET ${TARGETS})
        add_executable(
                ${TARGET} ${TARGET}.cpp ${PROTO_SRC}
        )
        target_compile_features(
                ${TARGET} PRIVATE cxx_std_20
        )
        target_include_directories(
                ${TARGET} PRIVATE
                ${Protobuf_INCLUDE_DIRS}
                ${PROTO_GEN_DIR}
        )
        target_link_libraries(
                ${TARGET} PRIVATE
                brpc leveldb gflags ssl crypto gtest fmt spdlog cpprest etcd-cpp-api 
                amqpcpp ev pthread
                ${Protobuf_LIBRARIES}
        )
endforeach()