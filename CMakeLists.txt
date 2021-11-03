cmake_minimum_required(VERSION 3.16)
project(chomp)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

set(CMAKE_CXX_FLAGS "-O3 -march=native -Wall -Wextra")
add_executable(chomp
        main.cpp
        position.cpp
        position.hpp
        store.cpp
        store.hpp
        datastructs.cpp
        datastructs.hpp
        parallel_hashmap/meminfo.h
        parallel_hashmap/phmap.h
        parallel_hashmap/phmap_bits.h
        parallel_hashmap/phmap_dump.h
        parallel_hashmap/phmap_utils.h
        parallel_hashmap/phmap_fwd_decl.h)

target_link_libraries(chomp -lpthread)