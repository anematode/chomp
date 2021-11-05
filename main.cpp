//
// Created by Timothy Herchen on 10/31/21.
//

#include "position.hpp"
#include <iostream>
#include <chrono>

int main () {
	const int dim = 6;
	using Position = Chomp::Position<dim>;

	int count = 0;
	auto test = [&] (Position& p) -> void {
		count++;
	};

	int min_cnt = 0, max_cnt = 33;
	std::cout << Chomp::time_function([&] {
		Position::positions_with_tiles(min_cnt, max_cnt, test, dim, dim);
		std::cout << "Count: " << count << '\n';
	}) << '\n';

	std::cout << Chomp::time_function([&] {
		int sum = 0;
		for (int i = min_cnt; i <= max_cnt; ++i) {
			sum += Chomp::count_positions(i, i, dim, dim);
		}
		std::cout << "Count: " << sum << '\n';
	}) << '\n';

	std::cout << Chomp::time_function([&] {
		std::cout << "Count: " << Chomp::count_positions(min_cnt, max_cnt, dim, dim) << '\n';
	}) << '\n';

}
