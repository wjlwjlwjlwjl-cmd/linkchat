project(linkchat_jsoncpp)
cmake_minimum_required(VERSION 3.22.1)

add_executable(
    main main.cc
)
target_link_libraries(
    main PRIVATE jsoncpp
)
