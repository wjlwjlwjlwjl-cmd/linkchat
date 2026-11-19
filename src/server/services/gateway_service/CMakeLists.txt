cmake_minimum_required(VERSION 3.22.1)
project(linkchat-gateway)

set(CMAKE_CXX_STANDARD 17) # websocketpp 不支持20
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(gateway_bin src/gateway.cc)

add_custom_command(
    TARGET gateway_bin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/conf
    $<TARGET_FILE_DIR:gateway_bin>/conf
)
add_custom_command(TARGET gateway_bin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    ${BIN_DIR}/gateway/conf
)
add_custom_command(TARGET gateway_bin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE_DIR:gateway_bin>/conf/settings.conf
    ${BIN_DIR}/gateway/conf
)
add_custom_command(TARGET gateway_bin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:gateway_bin>
    ${BIN_DIR}/gateway
)


target_link_libraries(
    gateway_bin PUBLIC
    adapters linkchat_proto
    linkchat_common
)
