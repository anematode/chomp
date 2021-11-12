//
// Created by Timothy Herchen on 11/7/21.
//

#ifndef CHOMP_BASE_ATLAS_HPP
#define CHOMP_BASE_ATLAS_HPP

#include "base_position.hpp"
#include <atomic>

namespace Chomp {
	// Whether a position is winning
	enum class WINNING {
		NO,
		YES,
		UNKNOWN
	};

	struct PositionInfo {
		WINNING is_winning = WINNING::UNKNOWN;
		// Distance to end is stored as an integer, -1 if unknown
		int distance_to_end = -1;

		std::string to_string();
	};

	std::ostream& operator<<(std::ostream&, PositionInfo);

	// General interface for how positions are stored
	template <int MAX_HEIGHT>
	class BaseAtlas {
	public:
		using Position = BasePosition<MAX_HEIGHT>;

		// whether a given position's information is known
		virtual bool is_position_known(Position& p) {
			return (p.width() < _bound_width && p.height() < _bound_height &&
				p.square_count() <= _max_square_count && p.square_count() >= _min_square_count);
		}

		virtual PositionInfo get_position_info(Position &p) const = 0;
		virtual PositionInfo get_position_info(Position &&p) const { return get_position_info(p); }
		virtual int known_losing_positions() { return _total_losing_positions; }
		virtual int known_positions() { return _total_known_positions; }
		virtual int known_winning_positions() {
			return (_total_known_positions != -1 && _total_losing_positions != -1) ?
			(_total_known_positions - _total_losing_positions) : -1;
		}

		// Store information about a given position
		virtual void store_position_info(Position &p, PositionInfo info) = 0;
		virtual void store_position_info(Position &&p, PositionInfo info) { store_position_info(p, info); }

		/*template <Lambda>
		virtual void for_each_known_losing_position(Lambda, int min_squares=-1, int max_squares=-1, int bound_width=-1, int bound_height=-1) = 0;*/
	public:
		// Signify that all positions with the given square count and bound width have been computed
		virtual void mark_positions_as_computed(int min_squares, int max_squares, int bound_width, int bound_height) {
			_min_square_count = std::max(min_squares, _min_square_count);
			_max_square_count = std::max(max_squares, _max_square_count);
			_bound_width = std::max(bound_width, _bound_width);
			_bound_height = std::max(bound_height, _bound_height);
		}

		// Hashed all positions with (width, height) less than this...
		int _bound_width = -1;
		int _bound_height = -1;

		// ... AND hashed all positions with these number of squares
		int _min_square_count = -1;
		int _max_square_count = -1;

		std::atomic<int> _total_known_positions = -1;
		std::atomic<int> _total_losing_positions = -1;
	};
}


#endif //CHOMP_BASE_ATLAS_HPP
