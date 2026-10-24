project(linkchat_adapter)
cmake_minimum_required(VERSION 3.22.1)

find_package(
        Protobuf REQUIRED
)

message(STATUS "Protobuf libraries: " ${Protobuf_LIBRARIES})
message(STATUS "Protobuf include: " ${Protobuf_INCLUDE_DIRS})
message(STATUS "Protobuf executable: " ${Protobuf_PROTOC_EXECUTABLE})

# 如果使用到了 proto，还需要注意proto代码文件、构建命令的维护

add_executable(
        main main.cpp
)

target_link_libraries(
        main PRIVATE fmt spdlog jsoncpp
)

target_include_directories(
        main PRIVATE ${CMAKE_INCLUDE_CURRENT_DIR} ${Protobuf_INCLUDE_DIRS}
)