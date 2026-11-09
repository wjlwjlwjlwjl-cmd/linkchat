cmake_minimum_required(VERSION 3.22.1)
project(linkchat_user)

add_subdirectory(src/entity)

file(GLOB_RECURSE SRCS "*.cc")

add_executable(
    user_service_bin ${SRCS}
)

add_custom_command(
    TARGET user_service_bin
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/conf
        $<TARGET_FILE_DIR:user_service_bin>/conf
)

target_include_directories(
    user_service_bin PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
    user_service_bin PRIVATE 
    user_entity linkchat_proto
    adapters util constants 
)