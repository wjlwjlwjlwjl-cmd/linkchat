cmake_minimum_required(VERSION 3.22.1)
project(linkchat_file)

file(GLOB_RECURSE SRCS "*.cpp")

add_executable(file_service_bin ${SRCS})

target_link_libraries(
    file_service_bin  PRIVATE
    file_proto gflags
)