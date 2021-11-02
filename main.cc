//
// Created by Timothy Herchen on 10/31/21.
//

#include <position.h>
#include <iostream>
#include <chrono>

int main () {
	Chomp::PositionFormatterOptions::set_default({ .tile_size=1, .sep=0 });

	using Position = Chomp::Position;

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	Chomp::hash_positions(81, 9, 9);

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
}