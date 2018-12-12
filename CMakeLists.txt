cmake_minimum_required(VERSION 3.1)
project(metro_web)

option(BUILD_PARALLEL_CODE "Build the parallel version" OFF)

set(CMAKE_CXX_STANDARD 11)

add_definitions(-DTINYPLY_IMPLEMENTATION=1)

if(BUILD_PARALLEL_CODE)
    add_definitions(-DIS_PARALLEL=1)
    find_package(Threads)
endif(BUILD_PARALLEL_CODE)

include_directories(external)

add_executable(metro_web
        main.cpp)

if(BUILD_PARALLEL_CODE)
    target_link_libraries(metro_web ${CMAKE_THREAD_LIBS_INIT})
endif(BUILD_PARALLEL_CODE)