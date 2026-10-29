find_package(Protobuf REQUIRED)

message(STATUS "Protobuf libraries: ${Protobuf_LIBRARIES}")
message(STATUS "Protobuf include: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf executable: ${Protobuf_PROTOC_EXECUTABLE}")

# base.proto
add_library(base_proto STATIC)
target_sources(base_proto PRIVATE base.proto)

protobuf_generate(
    TARGET base_proto
    LANGUAGE cpp
    PROTOC_OPTIONS --experimental_allow_proto3_optional  # <--- 修正为复数
)

# 暴露生成的头文件路径
target_include_directories(base_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
# 链接 protobuf 库
target_link_libraries(base_proto PUBLIC protobuf::libprotobuf)