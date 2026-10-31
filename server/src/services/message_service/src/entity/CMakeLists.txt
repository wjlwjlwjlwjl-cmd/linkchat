cmake_minimum_required(VERSION 3.22.1)
project(linkchat_message_storage_odb)

find_program(ODB_COMPILER odb REQUIRED)

set(persistent_hdr message_entity.h)

set(generated_srcs "")
set(generated_hdrs "")

foreach(header ${persistent_hdr})
    get_filename_component(base ${header} NAME_WE)

    set(gen_cxx ${CMAKE_CURRENT_BINARY_DIR}/${base}-odb.cxx)
    set(gen_hxx ${CMAKE_CURRENT_BINARY_DIR}/${base}-odb.hxx)

    add_custom_command(
        OUTPUT ${gen_cxx} ${gen_hxx}
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