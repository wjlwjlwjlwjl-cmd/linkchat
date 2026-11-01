cmake_minimum_required(VERSION 3.22.1)
project(linkchat_message_storage_entity)

find_program(ODB_COMPILER odb REQUIRED)

set(persistent_hdr message_entity.h)

set(generated_srcs "")
set(generated_hdrs "")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/sql/.keep
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/sql
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_SOURCE_DIR}/sql/.keep
)

foreach(header ${persistent_hdr})
    get_filename_component(base ${header} NAME_WE)

    set(gen_cxx ${CMAKE_CURRENT_BINARY_DIR}/${base}-odb.cxx)
    set(gen_hxx ${CMAKE_CURRENT_BINARY_DIR}/${base}-odb.hxx)
    set(gen_sql ${CMAKE_CURRENT_BINARY_DIR}/${base}.sql)
    set(dst_sql ${CMAKE_CURRENT_SOURCE_DIR}/sql/${base}.sql)

    add_custom_command(
        OUTPUT ${gen_cxx} ${gen_hxx} ${gen_sql}
        COMMAND ${ODB_COMPILER} 
            -d mysql
            --std c++11
            --generate-query
            --generate-schema
            --profile boost/date-time
            --output-dir ${CMAKE_CURRENT_BINARY_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/${header}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${header}
    )

    add_custom_command(
        OUTPUT ${dst_sql}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${gen_sql} ${dst_sql}
    )

    list(APPEND generated_hdrs ${gen_hxx})
    list(APPEND generated_srcs ${gen_cxx})
endforeach()

add_library(
    message_entity STATIC ${generated_srcs} ${generated_hdrs}
)
target_include_directories(
    message_entity PUBLIC 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
    message_entity PUBLIC 
    odb odb-boost odb-mysql pthread
)