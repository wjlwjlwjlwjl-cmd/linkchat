cmake_minimum_required(VERSION 3.22.1)
project(linkchat_file)

file(GLOB_RECURSE SRCS "*.cc")

add_executable(file_service_bin ${SRCS})

add_custom_command(TARGET file_service_bin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/conf
    $<TARGET_FILE_DIR:file_service_bin>/conf
)

add_custom_command(TARGET file_service_bin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    ${BIN_DIR}/file_service/conf
)
add_custom_command(TARGET file_service_bin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE_DIR:file_service_bin>/conf/settings.conf
    ${BIN_DIR}/file_service/conf
)
add_custom_command(TARGET file_service_bin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:file_service_bin>
    ${BIN_DIR}/file_service
)

target_include_directories(
    file_service_bin PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
    file_service_bin PRIVATE
    linkchat_proto gflags adapters util linkchat_common
)