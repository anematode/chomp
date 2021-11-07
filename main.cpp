//
// Created by Timothy Herchen on 10/31/21.
//

#include "atlas.hpp"
#include <iostream>
#include <chrono>

int main () {
	Chomp::PositionFormatOptions::set_default("austere");

	Chomp::Atlas<50> atlas;
	using Position = Chomp::Atlas<50>::Position;

	Position p{2, 2, 1};

	std::cout << atlas.get_position_hash_location(p).first << '\n';
}
