//
// Created by Timothy Herchen on 10/31/21.
//

#include "position_atlas.hpp"
#include <iostream>
#include <chrono>

int main () {
	Chomp::PositionFormatOptions::set_default("austere");

	using Position = Chomp::Position<15>;
	std::cout << Position{2, 2, 1}.cut(1,1).to_string();
}
