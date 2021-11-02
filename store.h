
#include <position.h>

namespace store {
	void write_map(const std::unordered_map<uint64_t, Chomp::PositionInfo>& map, const char* filename);
	void read_map(std::unordered_map<uint64_t, Chomp::PositionInfo>& map, const char* filename);
}