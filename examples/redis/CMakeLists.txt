cmake_minimum_required(VERSION 3.22.1)

add_executable(main main.cpp)

target_link_libraries(
    main PRIVATE hiredis redis++ pthread
)