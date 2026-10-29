project(linkchat_proto)
cmake_minimum_required(VERSION 3.22.1)

find_package(Protobuf REQUIRED)

message(STATUS "Protobuf libraries: " ${Protobuf_LIBRARIES})
message(STATUS "Protobuf include: " ${Protobuf_INCLUDE_DIRS})
message(STATUS "Protobuf executable: " ${Protobuf_PROTOC_EXECUTABLE})

set(proto_file ${CMAKE_SOURCE_DIR}/test.proto)
set(proto_src ${CMAKE_CURRENT_BINARY_DIR}/test.pb.cc)
set(proto_hdr ${CMAKE_CURRENT_BINARY_DIR}/test.pb.h)

add_custom_command(
    OUTPUT ${proto_src} ${proto_hdr}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --cpp_out=${CMAKE_BINARY_DIR}
         -I ${CMAKE_SOURCE_DIR}
         ${proto_file}
    DEPENDS ${proto_file}
)

add_executable(
    main main.cc ${proto_src}
)
target_include_directories(
    main PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${Protobuf_INCLUDE_DIRS}
)
target_link_libraries(
    main PRIVATE ${Protobuf_LIBRARIES}
)
