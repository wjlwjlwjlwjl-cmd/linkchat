cmake_minimum_required(VERSION 3.22.1)
project(linkchat_brpc)


# Protobuf
find_package(Protobuf REQUIRED)

message(STATUS "Protobuf libraries: ${Protobuf_LIBRARIES}")
message(STATUS "Protobuf include: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf executable: ${Protobuf_PROTOC_EXECUTABLE}")

set(PROTO_FILE ${CMAKE_SOURCE_DIR}/echo.proto)
set(PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(PROTO_SRC ${PROTO_GEN_DIR}/echo.pb.cc)
set(PROTO_HDR ${PROTO_GEN_DIR}/echo.pb.h)

add_custom_command(
        OUTPUT ${PROTO_SRC} ${PROTO_HDR}
        COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        ARGS --cpp_out=${PROTO_GEN_DIR}
                -I ${CMAKE_SOURCE_DIR}
                ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        VERBATIM
)
add_custom_target(proto_generate DEPENDS ${PROTO_SRC})

# ODB
find_program(ODB_COMPILER odb REQUIRED)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(ODB_GENERATE_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(PERSISTENT_HEADERS person.hxx)
set(ODB_GENERATE_SOURCES "")
foreach(header ${PERSISTENT_HEADERS})
        get_filename_component(base ${header} NAME_WE)
        set(gen_hxx ${ODB_GENERATE_DIR}/${base}-odb.hxx)
        set(gen_cxx ${ODB_GENERATE_DIR}/${base}-odb.cxx)
        add_custom_command(
                OUTPUT ${gen_hxx} ${gen_cxx}
                COMMAND ${ODB_COMPILER}
                        -d mysql
                        --std c++11
                        --generate-query
                        --generate-schema
                        --profile boost/date-time
                        --output-dir ${ODB_GENERATE_DIR}
                        ${CMAKE_CURRENT_SOURCE_DIR}/${header}
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${header}
        )
        list(APPEND ODB_GENERATE_SOURCES ${gen_cxx})
endforeach()
add_custom_target(odb_generate DEPENDS ${ODB_GENERATE_SOURCES})

# Code Building
set(TARGETS rpc_test etcd_registry_test etcd_discovery_test mq_test)
set(FRAMEWORK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
foreach(TARGET ${TARGETS})
        add_executable(
                ${TARGET} ${TARGET}.cpp ${PROTO_SRC} ${ODB_GENERATE_SOURCES}
        )
        add_dependencies(
                ${TARGET} proto_generate odb_generate
        )
        target_compile_features(
                ${TARGET} PRIVATE cxx_std_20
        )
        target_include_directories(
                ${TARGET} PRIVATE
                ${Protobuf_INCLUDE_DIRS}
                ${PROTO_GEN_DIR}
                ${ODB_GENERATE_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}
                ${FRAMEWORK_SOURCE_DIR}
        )
        target_link_libraries(
                ${TARGET} PRIVATE
                brpc leveldb gflags ssl crypto gtest fmt spdlog cpprest etcd-cpp-api 
                amqpcpp ev pthread hiredis redis++ odb odb-boost odb-mysql jsoncpp
                cpr elasticlient
                ${Protobuf_LIBRARIES}
        )
endforeach()