cmake_minimum_required(VERSION 3.22.1)
project(linkchat_es)

add_executable(
    main main.cc
)

target_link_libraries(
    main PRIVATE jsoncpp cpr elasticlient
)
