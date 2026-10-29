project(linkchat_gflags)
cmake_minimum_required(VERSION 3.22.1)
add_executable(
    main main.cc
)
find_package(
    gflags REQUIRED
)
target_link_libraries(
    main PRIVATE gflags
)
