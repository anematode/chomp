#include "base_atlas.hpp"

namespace Chomp {
	std::string winning_to_string(WINNING w) {
		using W = WINNING;

		switch (w) {
			case W::YES:
				return "yes";
			case W::NO:
				return "no";
			case W::UNKNOWN:
				return "unknown";
		}
	}

	std::string PositionInfo::to_string() {
		return "{ winning: " + winning_to_string(is_winning) + ", dte: " + std::to_string(distance_to_end) + " }";
	}

	std::ostream& operator<< (std::ostream& os, PositionInfo info) {
		return os << info.to_string();
	}
}