cmake_minimum_required(VERSION 3.22.1)
project(linkchat_brpc)

find_package(Protobuf REQUIRED)

message(STATUS "Protobuf libraries: ${Protobuf_LIBRARIES}")
message(STATUS "Protobuf include: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf executable: ${Protobuf_PROTOC_EXECUTABLE}")

set(proto_file ${CMAKE_SOURCE_DIR}/echo.proto)
# 统一用 CURRENT_BINARY_DIR，和 --cpp_out 保持一致
set(proto_gen_dir ${CMAKE_CURRENT_BINARY_DIR})
set(proto_src ${proto_gen_dir}/echo.pb.cc)
set(proto_hdr ${proto_gen_dir}/echo.pb.h)

# 用一个 custom target 统一驱动 proto 生成，保证只生成一次、且先于编译
add_custom_command(
    OUTPUT ${proto_src} ${proto_hdr}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --cpp_out=${proto_gen_dir}
    -I ${CMAKE_SOURCE_DIR}
    ${proto_file}
    DEPENDS ${proto_file}
    COMMENT "Generating protobuf sources for echo.proto"
    VERBATIM
)

add_custom_target(echo_proto_gen DEPENDS ${proto_src} ${proto_hdr})

set(all_targets echo_server echo_client)

foreach(target_name ${all_targets})
    add_executable(
        ${target_name} ${target_name}.cc ${proto_src}
    )
    # 确保先构建 proto 生成目标
    add_dependencies(${target_name} echo_proto_gen)
    target_include_directories(
        ${target_name} PRIVATE
        ${Protobuf_INCLUDE_DIRS}
        ${proto_gen_dir}
        ${CMAKE_SOURCE_DIR}
    )
    target_link_libraries(
        ${target_name} PRIVATE
        brpc leveldb gflags ssl crypto
        ${Protobuf_LIBRARIES}
    )
endforeach()
