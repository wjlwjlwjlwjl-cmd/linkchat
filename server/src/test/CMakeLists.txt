cmake_minimum_required(VERSION 3.22.1)
project(linkchat_brpc)

# Protobuf
find_package(Protobuf REQUIRED)

message(STATUS "Protobuf libraries: ${Protobuf_LIBRARIES}")
message(STATUS "Protobuf include: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf executable: ${Protobuf_PROTOC_EXECUTABLE}")

set(proto_file ${CMAKE_SOURCE_DIR}/echo.proto)
set(proto_gen_dir ${CMAKE_CURRENT_BINARY_DIR})
set(proto_src ${proto_gen_dir}/echo.pb.cc)
set(proto_hdr ${proto_gen_dir}/echo.pb.h)

add_custom_command(
    OUTPUT ${proto_src} ${proto_hdr}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --cpp_out=${proto_gen_dir}
         -I ${CMAKE_SOURCE_DIR}
         ${proto_file}
    DEPENDS ${proto_file}
    VERBATIM
)
add_custom_target(proto_generate DEPENDS ${proto_src})

# ODB
find_program(ODB_COMPILER odb REQUIRED)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(odb_generate_dir ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(persistent_headers person.h)
set(odb_generate_sources "")
foreach(header ${persistent_headers})
    get_filename_component(base ${header} NAME_WE)
    set(gen_hxx ${odb_generate_dir}/${base}-odb.hxx)
    set(gen_cxx ${odb_generate_dir}/${base}-odb.cxx)
    add_custom_command(
        OUTPUT ${gen_hxx} ${gen_cxx}
        COMMAND ${ODB_COMPILER}
                -d mysql
                --std c++11
                --generate-query
                --generate-schema
                --profile boost/date-time
                --output-dir ${odb_generate_dir}
                ${CMAKE_CURRENT_SOURCE_DIR}/${header}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${header}
    )
    list(APPEND odb_generate_sources ${gen_cxx})
endforeach()
add_custom_target(odb_generate DEPENDS ${odb_generate_sources})

# Code Building
# set(targets rpc_test etcd_registry_test etcd_discovery_test mq_test elastic_test mail_test)
set(targets oss_test)
set(framework_source_dir ${CMAKE_CURRENT_SOURCE_DIR}/..)
foreach(target ${targets})
    add_executable(
        ${target} ${target}.cc ${proto_src} ${odb_generate_sources}
    )
    add_dependencies(
        ${target} proto_generate odb_generate
    )
    target_compile_features(
        ${target} PRIVATE cxx_std_20
    )
    target_include_directories(
        ${target} PRIVATE
        ${Protobuf_INCLUDE_DIRS}
        ${proto_gen_dir}
        ${odb_generate_dir}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${framework_source_dir}
    )
    target_link_libraries(
        ${target} PRIVATE
        alibabacloud-oss-cpp-sdk
        brpc leveldb gflags ssl crypto gtest fmt spdlog cpprest etcd-cpp-api
        amqpcpp ev pthread hiredis redis++ odb odb-boost odb-mysql jsoncpp
        cpr elasticlient curl
        ${Protobuf_LIBRARIES}
    )
endforeach()
