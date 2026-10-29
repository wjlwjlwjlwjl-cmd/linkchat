cmake_minimum_required(VERSION 3.22.1)
project(linkchat_etcd)

set(all_targets put get)

foreach(target ${all_targets})
    add_executable(
        ${target} ${target}.cc
    )
    target_link_libraries(
        ${target} cpprest etcd-cpp-api
    )
endforeach()
