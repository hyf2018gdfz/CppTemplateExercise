cmake_minimum_required(VERSION 3.16)
project(DataStructureTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Debug 模式：带调试信息、全部警告、无优化
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -O0")
# Release 模式：二级优化
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
file(GLOB_RECURSE DS_HEADERS "${SRC_DIR}/*.hpp" "${SRC_DIR}/common.h" "${SRC_DIR}/test.h" "${TEST_DIR}/testcase.h")
set(TEST_DIR "${CMAKE_SOURCE_DIR}/test")
file(GLOB TEST_SOURCES "${TEST_DIR}" "${TEST_DIR}/t*.cpp")

set(MAIN_TEST "${CMAKE_SOURCE_DIR}/test/main.cpp")

add_executable(mytest
    ${MAIN_TEST}
    ${TEST_SOURCES}
    ${DS_HEADERS}
)

target_include_directories(mytest PUBLIC ${SRC_DIR})

# enable_testing()
# add_test(NAME all_tests COMMAND mytest)