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
        sparsehash/dense_hash_map.hpp
        sparsehash/dense_hash_table.hpp
        sparsehash/hashtable_common.hpp
        sparsehash/libc_allocator_with_realloc.hpp)
