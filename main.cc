//
// Created by Timothy Herchen on 10/31/21.
//

#include <position.h>
#include <iostream>
#include <array>
#include <algorithm>

int main () {
	Chomp::PositionFormatterOptions::set_default({ .tile_size=1, .sep=0 });
	auto p = Chomp::Position::starting_rectangle(6, 6);

	std::cout << p.list() << '\n';
	p = p.cut({ 5, 5 });
	std::cout << p.list() << '\n';
	p = p.cut({ 4, 5 });
	std::cout << p;
	std::cout << p.list() << '\n';
	p = p.cut({ 5, 4 });
	std::cout << p;
	std::cout << p.list() << '\n';
	p = p.cut({ 3, 2 });
	std::cout << p;
	std::cout << p.list() << '\n';
	p = p.cut({ 0, 0 });
	std::cout << p;

	/*constexpr int max_height = 10;
	using Position = Chomp::Position<max_height>;

	Chomp::get_positions_with_n_tiles<max_height>(7, [&] (const Position& p) {
		std::cout << p.to_string({ .tile_size = 1, .sep = 0 }) << "-----\n";
	}, 3, 3);

	Position p = Position::starting_rectangle(5, 5);

	std::cout << p.cut({ 2, 3 });*/
}