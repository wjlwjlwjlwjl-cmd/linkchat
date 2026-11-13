cmake_minimum_required(VERSION 3.22.1)
project(linkchat_transmite)

file(GLOB_RECURSE SRCS "*.cc")

add_executable(
    transmite_bin ${SRCS}
)

add_custom_command(
    TARGET transmite_bin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/conf
        $<TARGET_FILE_DIR:transmite_bin>/conf
)

target_include_directories(
    transmite_bin PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
    transmite_bin PUBLIC
    linkchat_proto linkchat_entity
    adapters util constants
)