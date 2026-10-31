cmake_minimum_required(VERSION 3.22.1)
project(linkchat_message_storage)

add_subdirectory(src/entity)

file(GLOB_RECURSE SRCS "*.cc")

add_executable(
    message_storage_bin ${SRCS}
)

add_custom_command(
    TARGET message_storage_bin
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/conf
        $<TARGET_FILE_DIR:message_storage_bin>/conf
)

target_include_directories(
    message_storage_bin PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
    message_storage_bin PRIVATE 
    message_entity file_proto
    message_proto adapters util constants 
)