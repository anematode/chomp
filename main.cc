//
// Created by Timothy Herchen on 10/31/21.
//

#include <position.h>
#include <iostream>
#include <chrono>
#include <cstdio>

int main () {
	constexpr int dimension = 50;

	FILE* out = fopen("files/out.txt", "w");

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	Chomp::hash_positions(dimension, dimension, dimension);
	Chomp::store("files/100.bin");

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
}
