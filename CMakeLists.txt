cmake_minimum_required(VERSION 3.6)
project(mmap_wrapper)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        main.cpp)

add_executable(mmap_wrapper ${SOURCE_FILES})