cmake_minimum_required(VERSION 3.22.1)
project(linkchat_amqp)

set(targets amqp_pub amqp_con)

foreach(target ${targets})
    add_executable(${target} ${target}.cc)
    target_link_libraries(${target} PRIVATE amqpcpp ev pthread)
endforeach()
