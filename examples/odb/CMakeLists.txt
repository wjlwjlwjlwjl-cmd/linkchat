cmake_minimum_required(VERSION 3.22.1)
project(linkchat_odb)

# 找到 ODB 编译器
find_program(ODB_COMPILER odb REQUIRED)

# 指定持久化头文件
set(persistent_headers person.h)

# 列表，用来存放后续生成的源文件（*.cxx）
set(generated_sources "")
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated)
foreach(header ${persistent_headers})
    get_filename_component(base ${header} NAME_WE) # 获取部分文件名，此处为获取不带扩展名的文件名

    set(gen_cxx ${CMAKE_CURRENT_BINARY_DIR}/generated/${base}-odb.cxx)
    set(gen_hxx ${CMAKE_CURRENT_BINARY_DIR}/generated/${base}-odb.hxx)

    add_custom_command(
        OUTPUT ${gen_cxx} ${gen_hxx}
        COMMAND ${ODB_COMPILER}
            -d mysql
            --std c++11
            --generate-query
            --generate-schema
            --profile boost/date-time
            --output-dir ${CMAKE_CURRENT_BINARY_DIR}/generated
            ${CMAKE_CURRENT_SOURCE_DIR}/${header}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${header}
    )

    list(APPEND generated_sources ${gen_cxx})
endforeach()

add_executable(
    main main.cc ${generated_sources}
)

target_include_directories(
    main PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    main PRIVATE odb odb-boost odb-mysql pthread
)
