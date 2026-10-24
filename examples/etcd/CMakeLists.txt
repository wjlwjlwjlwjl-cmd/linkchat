CMAKE_MINIMUM_REQUIRED(VERSION 3.22.1)
project(linkchat_etcd)

set (ALL_TARGETS put get)

foreach(target ${ALL_TARGETS})
    add_executable(
        ${target} ${target}.cc
    )
    target_link_libraries(
        ${target} cpprest etcd-cpp-api
    )
endforeach()