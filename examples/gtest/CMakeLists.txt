project(linkchat_gtest)
CMAKE_MINIMUM_REQUIRED(VERSION 3.22.1)

# ubuntu 的 libgtest-dev 确认规范，缺少若干 *.cmake 文件，导致cmake压根找不到库文件

add_executable(
        main main.cpp
)
target_link_libraries(
        main PRIVATE gtest
)