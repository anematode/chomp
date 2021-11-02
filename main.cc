//
// Created by Timothy Herchen on 10/31/21.
//

#include <position.h>
#include <iostream>
#include <chrono>
#include <cstdio>

int main () {
	// MAX_HEIGHT: 100, dimension: 80, NUM_THREADS: 8, BATCH_SIZE: 1000000
	// With canonical hashing: hash_positions took 90.02 seconds (Tim's computer)
	// With simple hashing: hash_positions took 200.3 seconds (Tim's computer)
	// Canonical hashing, BATCH_SIZE: 10000000, hash_positions took 102.4 seconds (Tim's computer)

	constexpr int dimension = 80;
	using namespace Chomp;

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	Chomp::hash_positions(dimension, dimension, dimension, { .compute_dte=false });

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

	//Chomp::store_positions("/Users/timoothy/Documents/GitHub/chomp/files/18by18.bin");
}
