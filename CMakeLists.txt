cmake_minimum_required(VERSION 3.6)
project(mmap_wrapper)

set(CMAKE_CXX_STANDARD 11)
include_directories(src)

set(SOURCE_FILES
        test/test.cpp src/FileMap.cpp src/FileMap.h)

add_executable(mmap_wrapper ${SOURCE_FILES})