cmake_minimum_required(VERSION 3.22.1)
project(linkchat_file)

find_package(Protobuf REQUIRED)

message(STATUS "Protobuf libraries: ${Protobuf_LIBRARIES}")
message(STATUS "Protobuf include: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf executable: ${Protobuf_PROTOC_EXECUTABLE}")

set(proto_file ${CMAKE_SOURCE_DIR}/file.proto)
set(proto_gen_dir ${CMAKE_CURRENT_BINARY_DIR})
set(proto_src ${proto_gen_dir}/echo.pb.cc)
set(proto_hdr ${proto_gen_dir}/echo.pb.h)

add_executable(
    main main.cpp
)

target_link_libraries(
    main PRIVATE gflags spdlog protobuf
)