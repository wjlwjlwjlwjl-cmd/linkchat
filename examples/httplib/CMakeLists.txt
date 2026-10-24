project(linkchat_httplib)
CMAKE_MINIMUM_REQUIRED(VERSION 3.22.1)

add_executable(
        main main.cc
)
target_link_libraries(
        main PRIVATE pthread
)