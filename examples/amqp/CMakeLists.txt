cmake_minimum_required(VERSION 3.22.1)
project(linkchat_amqp)

set (TARGETS amqp_pub amqp_con)

foreach(TARGET ${TARGETS})
    add_executable(${TARGET} ${TARGET}.cc)
    target_link_libraries(${TARGET} PRIVATE amqpcpp ev pthread)
endforeach()