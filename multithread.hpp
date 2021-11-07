#pragma once

#include "base_position.hpp"

namespace Chomp {
	namespace threaded {
		template<int MAX_HEIGHT, Lambda callback>
		bool positions_with_tiles(int min, int max, Lambda callback, int bound_width=-1, int bound_height=-1, bool only_canonical=false,
														int num_threads=1, size_t batch_size=1000000) {
			BasePosition<MAX_HEIGHT>::positions_with_tiles()
		}
	}
}