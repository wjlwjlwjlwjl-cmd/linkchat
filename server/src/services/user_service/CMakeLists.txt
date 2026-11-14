cmake_minimum_required(VERSION 3.22.1)
project(linkchat_user)

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
add_custom_command(TARGET user_service_bin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    ${BIN_DIR}/user_service/conf
)
add_custom_command(TARGET user_service_bin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE_DIR:user_service_bin>/conf/settings.conf
    ${BIN_DIR}/user_service/conf
)
add_custom_command(TARGET user_service_bin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:user_service_bin>
    ${BIN_DIR}/user_service
)

target_include_directories(
    user_service_bin PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
    user_service_bin PRIVATE 
    linkchat_entity linkchat_proto linkchat_common
    adapters util constants 
)