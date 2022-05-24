//
// Created by Timothy Herchen on 10/31/21.
//

#include "atlas.hpp"
#include "utils.hpp"
#include <iostream>
#include <chrono>

// Perf results:
// Hash positions up to 65 tiles
// Single-threaded, canonical hashing, no cut elimination, no dte, width-height-square_count-hash method: 12133 ms
// Single-threaded, canonical hashing, cut elimination, no dte, width-height-square_count-hash method: 8223 ms
// Single-threaded, canonical hashing, cut elimination, no dte, width-height-hash method: 7368 ms
// Hash positions up to 75 tiles
// Single-threaded, canonical hashing, cut elimination, no dte, width-height-hash method: 37850 ms
// Single-threaded, canonical hashing, cut elimination, no dte, width-height-square_count-hash method: 40024 ms

int main () {
	using namespace Chomp;
	Chomp::PositionFormatOptions::set_default("austere");

	Chomp::Atlas<3> atlas;

	atlas.hash_positions(-1, -1, 300, 3);

	//

	// #
	// ###
	// ####
}
