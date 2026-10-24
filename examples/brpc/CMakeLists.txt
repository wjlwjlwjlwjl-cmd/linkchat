cmake_minimum_required(VERSION 3.22.1)
project(linkchat_brpc)

find_package(Protobuf REQUIRED)

message(STATUS "Protobuf libraries: ${Protobuf_LIBRARIES}")
message(STATUS "Protobuf include: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf executable: ${Protobuf_PROTOC_EXECUTABLE}")

set(PROTO_FILE ${CMAKE_SOURCE_DIR}/echo.proto)
# 统一用 CURRENT_BINARY_DIR，和 --cpp_out 保持一致
set(PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(PROTO_SRC ${PROTO_GEN_DIR}/echo.pb.cc)
set(PROTO_HDR ${PROTO_GEN_DIR}/echo.pb.h)

# 用一个 custom target 统一驱动 proto 生成，保证只生成一次、且先于编译
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

add_custom_target(echo_proto_gen DEPENDS ${PROTO_SRC} ${PROTO_HDR})

set(ALL_TARGETS echo_server echo_client)

foreach(target_name ${ALL_TARGETS})
    add_executable(
            ${target_name} ${target_name}.cpp ${PROTO_SRC}
    )
    # 确保先构建 proto 生成目标
    add_dependencies(${target_name} echo_proto_gen)
    target_include_directories(
            ${target_name} PRIVATE
            ${Protobuf_INCLUDE_DIRS}
            ${PROTO_GEN_DIR}
            ${CMAKE_SOURCE_DIR}
    )
    target_link_libraries(
            ${target_name} PRIVATE
            brpc leveldb gflags ssl crypto
            ${Protobuf_LIBRARIES}
    )
endforeach()