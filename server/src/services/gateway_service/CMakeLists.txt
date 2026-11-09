cmake_minimum_required(VERSION 3.22.1)
project(linkchat-gateway)

add_executable(linkchat-gateway-bin src/gateway.cc)

add_custom_command(
    TARGET linkchat-gateway-bin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/conf
    $<TARGET_FILE_DIR:linkchat-gateway-bin>/conf
)


target_link_libraries(
    linkchat-gateway-bin PUBLIC
    adapters linkchat_proto
)
