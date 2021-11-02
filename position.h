//
// Created by Timothy Herchen on 10/31/21.
//

#ifndef CHOMP_POSITION_H
#define CHOMP_POSITION_H

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define INT_MAX std::numeric_limits<int>::max()

// Get the file and line number
#define FILE_LINE __FILE__ ":" STR(__LINE__) ": "

#define DEBUG 0

#include <ostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <initializer_list>
#include <iostream>
#include <type_traits>
#include <unordered_map>

namespace Chomp {
	// Globally defined max height
	constexpr int MAX_HEIGHT = 25;

	// Options for formatting to string
	struct PositionFormatterOptions
	{
		// Size of each tile in characters
		int tile_width = 3;
		int tile_height = 2;
		// If positive, used for tile_width/tile_height
		int tile_size = -1;

		// Minimum number of tiles to display in each direction
		int min_width = 3;
		int min_height = 3;

		// Separation in newlines and spaces, respectively, between tiles in each direction
		int horizontal_sep = 1;
		int vertical_sep = 1;
		// If positive, used for vertical_sep/horizontal_sep
		int sep = -1;

		// Character for when a tile is filled or not filled
		char tile_char = 'X';
		char empty_char = ' ';

		// Show labels
		bool show_labels = true;

		int get_horizontal_sep();
		int get_vertical_sep();
		int get_tile_width();
		int get_tile_height();

		static void set_default(PositionFormatterOptions opts);
	};

  inline PositionFormatterOptions default_formatter_options;

	struct PositionInfo {
		bool is_winning; // whether the position is winning
		int dte; // distance to game end, assuming optimal play
	};

	using Cut = std::pair<int, int>;

	/**
	 * Stores a given board position as an array of the number of tiles in each row, from bottom to top
	 * @tparam MAX_HEIGHT The tallest allowed board
	 */
	class Position {
	public:
		// Number of squares per row
		int rows[MAX_HEIGHT];
		// Number of non-zero rows
		int height;

		Position();
		Position(const std::initializer_list<int>&);
		Position(const Position&); // copy constructor

		void make_empty();

		bool is_legal() const;
		void normalize_height();

		std::string to_string(PositionFormatterOptions=default_formatter_options) const;
		std::string list() const;

		int get_height() const;
		int get_width() const;

		int square_count() const;
		uint64_t hash() const;

		Position cut (int row, int col) const;
		Position cut (Cut) const;

		void reflect_if_necessary();

		template <typename Lambda>
		void for_each_cut(Lambda) const;

		static Position starting_rectangle(int width, int height);
		static Position empty_position();

		std::vector<Cut> winning_cuts() const;
		int num_winning_cuts() const;

		PositionInfo info() const;
	};

	/**
	 * Get all positions with exactly n tiles, bounded by bound_width and bound_height (their dimensions are within those
	 * bounds)
	 * @tparam MAX_HEIGHT
	 * @tparam Lambda Type of callback function
	 * @param n Number of tiles
	 * @param callback Callback function accepting a single parameter, the position (as a const ref)
	 * @param bound_width -1 if unbounded; otherwise, the bound on the width
	 * @param bound_height -1 if unbounded; otherwise, the bound on the height, superseded by MAX_HEIGHT if necessary
	 */
	template<typename Lambda>
	void get_positions_with_n_tiles(int n, Lambda callback, int bound_width = -1, int bound_height = -1) {
		static_assert(std::is_invocable_v<Lambda, const Position &>,
		              FILE_LINE"Second parameter to get_positions_with_n_tiles must be a function that accepts Positions with the same MAX_HEIGHT");

		if (n < 0)
			throw new std::runtime_error(FILE_LINE
			                             "n must be a positive integer");

		if (bound_height == -1) bound_height = INT_MAX;
		if (bound_width == -1) bound_width = INT_MAX;

		// prevent OOB
		bound_height = std::min(bound_height, MAX_HEIGHT);

		// We start off with some position and modify it iteratively. We want the rows to sum to n, the number of rows to be
		// less than or equal to the bound_height, and each row to be less than the bound_width. If we are determining the
		// value of a given row r (indexing from 0 as usual), then if we give it a value of v, the maximum number of remaining
		// tiles to place is (bound_height - r) * v and the minimum number is 0. If we have a certain number of remaining
		// tiles to place, these bounds give us information on how many we should try.

		Position p; // position to manipulate and be passed to the lambda
		p.make_empty();
		int *rows = p.rows;

		// Index we are currently modifying
		int i = 0;

		// Tiles remaining to be placed
		int remaining = n;

		while (true) {
			int rows_remaining = bound_height - i;

			int current = rows[i]; // starts at 0
			remaining += current;

			// ceiling division of remaining and rows_remaining; we need to place at least this many squares
			int min_place = (remaining + rows_remaining - 1) / rows_remaining;

			// Upper bound on how many to place
			int max_place = std::min((i == 0) ? remaining : rows[i - 1], bound_width);

			if (current >= max_place || max_place < min_place) {
				// backtrack, though this shouldn't happen very often
				rows[i--] = 0;

				if (i == -1) break;
				continue;
			} else if (current < min_place) {
				current = min_place;
			} else {
				// min_place <= current < max_place
				current++;
			}

			rows[i] = current;
			remaining -= current;

			if (remaining == 0) { // If we've placed all tiles, set the height appropriately, callback
				p.height = (current == 0) ? i : (i + 1);
				callback(p);

				rows[i--] = 0;
				remaining += current;

				if (i == -1) break;
			} else {
				// Some tiles remain to be placed; continue if possible
				i = std::min(i + 1, bound_height - 1);
			}
		}
	}

	/**
	 * Get all positions with n tiles or less, bounded by bound_width and bound_height (their dimensions are within those
	 * bounds). The order in which the positions are given is guaranteed to be in order of number of squares.
	 * @tparam MAX_HEIGHT
	 * @tparam Lambda Type of callback function
	 * @param n Number of tiles
	 * @param callback Callback function accepting a single parameter, the position (as a const ref)
	 * @param bound_width -1 if unbounded; otherwise, the bound on the width
	 * @param bound_height -1 if unbounded; otherwise, the bound on the height, superseded by MAX_HEIGHT if necessary
	 */
	template<typename Lambda>
	void get_positions_with_tiles(int min, int max, Lambda callback, int bound_width = -1, int bound_height = -1) {
		for (int i = min; i < max; ++i) {
			if (DEBUG)
				std::printf("Getting positions with %i tiles\n", i);
			get_positions_with_n_tiles(i, callback, bound_width, bound_height);
		}
	}

	template <typename Lambda>
	void Position::for_each_cut(Lambda callback) const {
		for (int i = 0; i < height; ++i) {
			int cnt = rows[i];

			for (int col = 0; col < cnt; ++col) {
				callback({ i, col });
			}
		}
	}

	void hash_positions(int max_squares, int bound_width=-1, int bound_height=-1);
}

#endif //CHOMP_POSITION_H
