cmake_minimum_required(VERSION 3.16)
project(chomp)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")
add_executable(chomp
        main.cc
        position.cpp
        position.h
        store.cpp
        store.h)

target_link_libraries(chomp -lpthread)