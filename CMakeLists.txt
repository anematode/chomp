cmake_minimum_required(VERSION 3.19)
project(chomp)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

add_executable(chomp
        main.cc
        position.h)
