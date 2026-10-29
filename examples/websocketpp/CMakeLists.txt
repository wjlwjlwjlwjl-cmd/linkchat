project(linkchat_websocketpp)
cmake_minimum_required(VERSION 3.22.1)

add_executable(
    main main.cc
)
target_link_libraries(
    main PRIVATE pthread boost_system ssl crypto
)
