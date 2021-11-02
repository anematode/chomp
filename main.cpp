//
// Created by Timothy Herchen on 10/31/21.
//

#include "position.hpp"
#include <iostream>
#include <chrono>

int main (int argc, const char* argv[]) {
	// MAX_HEIGHT: 100, dimension: 80, NUM_THREADS: 8, BATCH_SIZE: 1000000
	// With canonical hashing: hash_positions took 90.02 seconds (Tim's computer)
	// With simple hashing: hash_positions took 200.3 seconds (Tim's computer)
	// Canonical hashing, BATCH_SIZE: 10000000, hash_positions took 102.4 seconds (Tim's computer)
	// With hashing including square count: 89.0 seconds
	// With google::dense_hash_map: 66.6 seconds
	// With phmap::parallel_flat_hash_map: 53.1 seconds

	constexpr int dimension = 120;
	using namespace Chomp;

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	Chomp::hash_positions(dimension, dimension, dimension, { .num_threads = std::stoi(argv[0]), .batch_size = std::stoi(argv[1]) });

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

	Chomp::store_positions("./files/120.bin");
}
