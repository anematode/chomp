//
// Created by Timothy Herchen on 10/31/21.
//

#include <position.h>
#include <iostream>
#include <chrono>
#include <cstdio>

int main () {
	Chomp::PositionFormatterOptions::set_default({ .tile_size=2, .sep=1 });

	using Position = Chomp::Position;

	constexpr int dimension = 52;

	FILE* out = fopen("files/out.txt", "w");

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	// Position p = Position({5,4,3,3,2});

	// fprintf(out, "%s\n", p.to_string().c_str());
	// p.reflect_if_necessary();
	// fprintf(out, "%s\n", p.to_string().c_str());

	Chomp::hash_positions(dimension, dimension, dimension);

	int winning_moves = 0, positions = 0;

	for (int n = 1; n <= dimension; ++n) {
		Chomp::get_positions_with_tiles(n, n+1, [&] (const Position& p) {
			//if (!p.info().is_winning) return;

			positions++;
			int c = p.num_winning_cuts();
			winning_moves += c;

			/*if (c > 3) {
				std::cout << p;
			}*/
		});

		std::printf("%i\t%f\n", n, winning_moves / (float) positions);
	}




	/*Chomp::get_positions_with_tiles(0, dimension, [&] (const Position& p) {
		if (!p.info().is_winning) {
			fprintf(out, "%s\n", p.to_string().c_str());
		}
	});

	fclose(out);

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;*/
}
