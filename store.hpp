
#include "position.hpp"

namespace Chomp {
	namespace store {
		void write_map(const map_type &map, const char *filename);

		void read_map(map_type &map, const char *filename);
	}
}