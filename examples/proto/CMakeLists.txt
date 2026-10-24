project(linkchat_jsoncpp)
CMAKE_MINIMUM_REQUIRED(VERSION 3.22.1)

find_package(Protobuf REQUIRED)

message(STATUS "Protobuf libraries: " ${Protobuf_LIBRARIES})
message(STATUS "Protobuf include: " ${Protobuf_INCLUDE_DIRS})
message(STATUS "Protobuf executable: " ${Protobuf_PROTOC_EXECUTABLE})

set(PROTO_FILE ${CMAKE_SOURCE_DIR}/test.proto)
set(PROTO_SRC ${CMAKE_CURRENT_BINARY_DIR}/test.pb.cc)
set(PROTO_HDR ${CMAKE_CURRENT_BINARY_DIR}/test.pb.h)

add_custom_command(
        OUTPUT ${PROTO_SRC} ${PROTO_HDR}
        COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        ARGS --cpp_out=${CMAKE_BINARY_DIR}
             -I ${CMAKE_SOURCE_DIR}
             ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
)

add_executable(
        main main.cc ${PROTO_SRC}
)
target_include_directories(
        main PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${Protobuf_INCLUDE_DIRS}
)
target_link_libraries(
        main PRIVATE ${Protobuf_LIBRARIES}
)