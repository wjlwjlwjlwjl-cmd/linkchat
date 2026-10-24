project(linkchat_brpc)
CMAKE_MINIMUM_REQUIRED(VERSION 3.22.1)

find_package(Protobuf REQUIRED)

message(STATUS "Protobuf libraries: " ${Protobuf_LIBRARIES})
message(STATUS "Protobuf include: " ${Protobuf_INCLUDE_DIRS})
message(STATUS "Protobuf executable: " ${Protobuf_PROTOC_EXECUTABLE})

set(PROTO_FILE ${CMAKE_SOURCE_DIR}/echo.proto)
set(PROTO_SRC ${CMAKE_CURRENT_BINARY_DIR}/echo.pb.cc)
set(PROTO_HDR ${CMAKE_CURRENT_BINARY_DIR}/echo.pb.h)

set(ALL_TARGETS echo_server echo_client)

add_custom_command(
        OUTPUT ${PROTO_SRC} ${PROTO_HDR}
        COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        ARGS --cpp_out=${CMAKE_BINARY_DIR}
        -I ${CMAKE_SOURCE_DIR}
        ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
)

foreach(target_name ${ALL_TARGETS})
    add_executable(
            ${target_name} ${target_name}.cpp ${PROTO_SRC}
    )
    target_include_directories(
            ${target_name} PRIVATE ${Protobuf_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}
    )
    target_link_libraries(
            ${target_name} PRIVATE brpc leveldb gflags ssl crypto protobuf
    )
endforeach()