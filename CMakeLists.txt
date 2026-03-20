cmake_minimum_required(VERSION 3.16)
project(DataStructureTests LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Debug 模式：带调试信息、全部警告、无优化
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -O0")
# Release 模式：二级优化
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

find_package(Threads REQUIRED)

# 1. 原有的数据结构测试
file(GLOB DS_HEADERS "${CMAKE_SOURCE_DIR}/src/*.hpp" "${CMAKE_SOURCE_DIR}/src/common.h" "${CMAKE_SOURCE_DIR}/src/test.h")
file(GLOB TEST_SOURCES "${CMAKE_SOURCE_DIR}/test/t*.cpp")
add_executable(mytest "${CMAKE_SOURCE_DIR}/test/main.cpp" ${TEST_SOURCES} ${DS_HEADERS})
target_include_directories(mytest PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(mytest PRIVATE Threads::Threads)

# 2. 引入 mymalloc 模块
add_subdirectory(src/mymalloc)

# 3. 引入 mymalloc 的测试模块
add_subdirectory(test/mymalloc)