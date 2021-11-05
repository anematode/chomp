#pragma once

#include "base_position.hpp"

namespace Chomp {
	class PositionInfo {
		bool is_winning;
		int dte;
		int id;
	};

	// Stores a set of positions. Generally an atlas will have all positions in a rectangle or all positions with a given
	// square count or less. Note that the atlas may not necessarily store all the positions; for space efficiency, the
	// losing positions may sometimes only be stored
	template<int MAX_HEIGHT>
	class Atlas {
		using Position = BasePosition<MAX_HEIGHT>;

	private:
		// Hashed all positions with (width, height) less than this
		int _bound_width = -1;
		int _bound_height = -1;

		// Hashed all positions with this square count or fewer
		int _max_square_count = -1;
	};
}