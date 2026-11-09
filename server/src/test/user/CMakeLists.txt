cmake_minimum_required(VERSION 3.22.1)
project(linkchat-user-test)

add_executable(
    user_test_bin test_user.cc
)

target_link_libraries(
    user_test_bin PUBLIC
    adapters linkchat_proto
)