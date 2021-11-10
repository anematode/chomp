//
// Created by Timothy Herchen on 10/31/21.
//

#include "atlas.hpp"
#include "utils.hpp"
#include <iostream>
#include <chrono>

// Perf results:
// Single-threaded

int main () {
	using namespace Chomp;
	Chomp::PositionFormatOptions::set_default("austere");

	const int dim = 65;

	Chomp::Atlas<dim> atlas;
	using Position = Chomp::Atlas<dim>::Position;

	//atlas.hash_positions(0, 4);

	Position p{1};
	Position p2{3, 2};

	std::cout << time_function([&] {
		atlas.hash_positions(1, dim, dim, dim);
	});

}
