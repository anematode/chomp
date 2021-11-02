
#include <position.h>

namespace Chomp {
	namespace store {
		void write_map(const std::vector<map_type*> &map, const char *filename);

		void read_map(std::vector<map_type*> &map, const char *filename);
	}
}