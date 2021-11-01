//
// Created by Timothy Herchen on 10/31/21.
//

#include <position.h>
#include <iostream>

int main () {
	Chomp::Position<5> p{4, 4, 3};

	std::cout << p.to_string();
	std::cout << Chomp::hash_position(p);
}