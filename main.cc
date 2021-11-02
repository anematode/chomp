//
// Created by Timothy Herchen on 10/31/21.
//

#include <position.h>
#include <iostream>
#include <chrono>
#include <cstdio>

int main () {
	constexpr int dimension = 100;

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	Chomp::hash_positions(dimension, dimension, dimension);

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

	Chomp::store_positions("/Users/timoothy/Documents/GitHub/chomp/files/100.bin");
}
