file(GLOB_RECURSE MYMALLOC_SRCS "*.cpp" "*.h" "*.hpp")

# 如果暂时没有 cpp 文件，强制指定语言
add_library(mymalloc SHARED ${MYMALLOC_SRCS})
set_target_properties(mymalloc PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(mymalloc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(mymalloc PRIVATE Threads::Threads)

add_library(mymalloc_static STATIC ${MYMALLOC_SRCS})
set_target_properties(mymalloc_static PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(mymalloc_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(mymalloc_static PRIVATE Threads::Threads)