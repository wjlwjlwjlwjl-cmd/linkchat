# 将所有持久化头文件编译到一个静态库 linkchat_entity 中

cmake_minimum_required(VERSION 3.22.1)
project(linkchat_entity)

find_program(ODB_COMPILER odb REQUIRED)

set(persistent_hdr
    user_entity.h
    message_entity.h
    chat_session.h
    chat_session_member.h
    friend_entity.h
    relation.h
)

set(generated_srcs "")
set(generated_hdrs "")
set(copied_sqls "")

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
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/sql
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${gen_sql} ${dst_sql}
        DEPENDS ${gen_sql}
        COMMENT "Copying ${base}.sql to entity/sql"
    )

    list(APPEND copied_sqls ${dst_sql})
    list(APPEND generated_hdrs ${gen_hxx})
    list(APPEND generated_srcs ${gen_cxx})
endforeach()

# 去执行拷贝
add_custom_target(
    linkchat_entity_sql ALL
    DEPENDS ${copied_sqls}
)

add_library(
    linkchat_entity STATIC ${generated_srcs}
)
target_include_directories(
    linkchat_entity PUBLIC 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
    linkchat_entity PUBLIC 
    odb odb-boost odb-mysql pthread
)