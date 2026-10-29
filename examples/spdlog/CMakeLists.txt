project(linkchat_spdlog)
cmake_minimum_required(VERSION 3.22.1)

add_executable(
    main main.cc
)
target_link_libraries(
    main PRIVATE spdlog fmt
)
