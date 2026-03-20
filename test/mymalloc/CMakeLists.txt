file(GLOB_RECURSE MYMALLOC_TEST_SRCS "*.cpp")

if(MYMALLOC_TEST_SRCS)
    add_executable(mymalloc_test ${MYMALLOC_TEST_SRCS})
    
    target_include_directories(mymalloc_test PUBLIC 
        ${CMAKE_SOURCE_DIR}/src/mymalloc 
        ${CMAKE_SOURCE_DIR}/src
    )
    
    # 强制链接静态库实现全局替换
    target_link_libraries(mymalloc_test PRIVATE 
        -Wl,--whole-archive mymalloc_static -Wl,--no-whole-archive
        Threads::Threads
    )
endif()