cmake_minimum_required(VERSION 3.1)
project(metro_web)

set(CMAKE_CXX_STANDARD 11)

add_definitions(-DTINYPLY_IMPLEMENTATION=1)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -Wall")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

include_directories(external)

add_executable(metro_web
        CMakeLists.txt
        main.cpp)
