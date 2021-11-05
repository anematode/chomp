//
// Created by Timothy Herchen on 10/31/21.
//

#include "position.hpp"
#include <iostream>

int main () {
	using Position = Chomp::Position<5>;

	int count = 5;
	auto test = [&] (Position p) -> void {
		count++;
	};

	Position::positions_with_n_tiles(6, test);
	std::cout << std::to_string(false);

}
