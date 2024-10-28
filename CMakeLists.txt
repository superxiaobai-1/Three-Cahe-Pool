cmake_minimum_required(VERSION 3.10)
project(TreeCacheMalloc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置调试信息
set(CMAKE_BUILD_TYPE Debug)

# 禁用优化
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES
    source/CentralCache.cpp
    source/PageCache.cpp
    source/thread_cache.cpp
    bench/UnitTest.cpp
    bench/Benchmark.cpp
)

add_library(tc_pool SHARED ${SOURCES})

target_link_libraries(tc_pool PRIVATE pthread)

add_subdirectory(bench)

install(TARGETS tc_pool
        DESTINATION ${CMAKE_SOURCE_DIR}/build/lib)