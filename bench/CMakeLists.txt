add_executable(tc_malloc Benchmark.cpp)

set(CMAKE_INSTALL_RPATH "/Users/liujie/work/tcmalloc-mini/tc//build/lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
target_link_libraries(tc_malloc tc_pool)
target_include_directories(tc_malloc PRIVATE ${CMAKE_SOURCE_DIR}/include)

install(TARGETS tc_malloc
        DESTINATION ${CMAKE_SOURCE_DIR}/build/bin)