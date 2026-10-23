CMAKE_MINIMUM_REQUIRED(VERSION 3.22.1)

add_executable(
        main main.cpp
)

find_package(
        gflags REQUIRED
)

target_link_libraries(
    main PRIVATE gflags
)
