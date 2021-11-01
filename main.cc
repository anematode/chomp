//
// Created by Timothy Herchen on 10/31/21.
//

#include <position.h>
#include <iostream>

int main () {
	constexpr int max_height = 10;
	using Position = Chomp::Position<max_height>;

	Chomp::get_positions_with_n_tiles<max_height>(7, [&] (Position p) {
		std::cout << p.to_string({ .tile_size = 1, .sep = 0 }) << "-----\n";
	}, 3, 3);
}