project(linkchat_jsoncpp)
CMAKE_MINIMUM_REQUIRED(VERSION 3.22.1)

add_executable(
        main main.cpp
)
target_link_libraries(
        main PRIVATE jsoncpp
)
