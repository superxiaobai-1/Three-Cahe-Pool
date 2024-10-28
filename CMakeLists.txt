cmake_minimum_required(VERSION 3.10)
project(TreeCacheMalloc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置调试信息
set(CMAKE_BUILD_TYPE Debug)

# 禁用优化
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

set(SOURCES
    CentralCache.cpp
    PageCache.cpp
    thread_cache.cpp
    UnitTest.cpp
    Benchmark.cpp
)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(tc_malloc11 ${SOURCES})

# 链接 pthread 库
target_link_libraries(tc_malloc11 PRIVATE pthread)
