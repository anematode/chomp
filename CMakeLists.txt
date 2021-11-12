cmake_minimum_required(VERSION 3.16)
project(chomp)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

set(CMAKE_CXX_FLAGS "-O3 -march=native -Wall -Wextra -std=c++20")
add_executable(chomp
        main.cpp
        utils.hpp
        base_position.cpp
        base_position.hpp
        base_atlas.hpp
        base_atlas.cpp
        atlas.hpp
        parallel_hashmap/meminfo.h
        parallel_hashmap/phmap.h
        parallel_hashmap/phmap_bits.h
        parallel_hashmap/phmap_dump.h
        parallel_hashmap/phmap_utils.h
        parallel_hashmap/phmap_fwd_decl.h)

target_link_libraries(chomp -lpthread)