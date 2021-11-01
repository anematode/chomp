//
// Created by Timothy Herchen on 10/31/21.
//

#ifndef CHOMP_POSITION_H
#define CHOMP_POSITION_H

#include <ostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <initializer_list>
#include <iostream>
#include <type_traits>

namespace Chomp {

	// Options for formatting to string
	struct PositionFormatterOptions
	{
		// Size of each tile in characters
		int tile_width = 3;
		int tile_height = 2;
		// If positive, used for tile_width/tile_height
		int tile_size = -1;

		// Minimum number of tiles to display in each direction
		int min_width = 1;
		int min_height = 1;

		// Separation in newlines and spaces, respectively, between tiles in each direction
		int horizontal_sep = 1;
		int vertical_sep = 1;
		// If positive, used for vertical_sep/horizontal_sep
		int sep = -1;


		// Character for when a tile is filled or not filled
		char tile_char = '#';
		char empty_char = ' ';

		// Show labels
		bool show_labels = true;

		int get_horizontal_sep();
		int get_vertical_sep();
		int get_tile_width();
		int get_tile_height();
	};

	int PositionFormatterOptions::get_vertical_sep() {
		return (sep < 0) ? vertical_sep : sep;
	}

	int PositionFormatterOptions::get_horizontal_sep() {
		return (sep < 0) ? horizontal_sep : sep;
	}

	int PositionFormatterOptions::get_tile_width() {
		return (tile_size < 0) ? tile_width : tile_size;
	}

	int PositionFormatterOptions::get_tile_height() {
		return (tile_size < 0) ? tile_height : tile_size;
	}

	/**
	 * Stores a given board position as an array of the number of tiles in each row, from bottom to top
	 * @tparam max_height The tallest allowed board
	 */
	template <int max_height>
	class Position {
	public:
		int rows[max_height];
		int height; // The number of non-zero rows

		Position();
		Position(const std::initializer_list<int>&);
		Position(const Position&); // copy constructor

		bool is_legal() const;
		void normalize_height();

		std::string to_string(PositionFormatterOptions={}) const;

		int getHeight() const;
		int getWidth() const;

		static Position starting_rectangle(int width, int height);
	};

	template <int max_height>
	uint64_t hash_position(const Position<max_height>& p) {
		// The hash function operates as follows: we add row[i], multiply by a large prime number p (mod 2^64), and proceed.
		// It isn't the greatest hash function, but it works.
		const int* rows = p.rows;

		uint64_t hash = 0;
		for (int i = 0; i < max_height; ++i) {
			hash += rows[i];
			hash *= 179424673L;
		}

		return hash;
	}

	template <int max_height>
	Position<max_height>::Position() {
		height = 0;
		for (int i = 0; i < max_height; ++i)
			rows[i] = 0;
	}

	template <int max_height>
	Position<max_height>::Position(const Position<max_height>& p) {
		height = p.height;
		for (int i = 0; i < max_height; ++i)
			rows[i] = p.rows[i];
	}

	template <int max_height>
	Position<max_height> Position<max_height>::starting_rectangle(int width, int height) {
		Position p;

		if (width < 0) throw std::runtime_error("Width cannot be negative");

		height = std::clamp(height, 0, max_height);

		p.height = height;
		std::fill_n(p.rows, height, width);
	}

	template <int max_height>
	bool Position<max_height>::is_legal() const {
		int prev = std::numeric_limits<int>::max(); // :)

		for (int i = 0; i < height; ++i) {
			int cnt = rows[i];
			if (cnt > prev || cnt < 0)
				return false;
			prev = cnt;
		}

		return true;
	}

	template <int max_height>
	void Position<max_height>::normalize_height() {
		// Set the height value to the appropriate value based on the rows array
		int i = 0;
		for (; i < max_height && rows[i]; ++i);

		height = i;
	}

	template <int max_height>
	Position<max_height>::Position(const std::initializer_list<int>& l) {
		int h = std::min(max_height, static_cast<int>(l.size()));

		// Is there a nicer way to do this?
		std::copy(l.begin(), l.begin() + h, rows);

		for (int i = h; i < max_height; ++i) rows[i] = 0;

		if (!is_legal()) throw std::runtime_error("Invalid position in initializer list");
		normalize_height();
	}

	template <int max_height>
	int Position<max_height>::getHeight() const {
		return height;
	}

	template <int max_height>
	int Position<max_height>::getWidth() const {
		return rows[0];
	}

	template <int max_height>
	std::string Position<max_height>::to_string(PositionFormatterOptions opts) const {
		// The basic unit of printing is a rectangle of size tile_width x tile_height, which we store as a sequence of chars
		// unbroken by newlines
		using namespace std;

		int tile_width = opts.get_tile_width();
		int tile_height = opts.get_tile_height();
		int tile_area = tile_width * tile_height;

		int horizontal_sep = opts.get_horizontal_sep();
		int vertical_sep = opts.get_vertical_sep();

		string empty_tile = string(tile_area, opts.empty_char);
		string filled_tile = string(tile_area, opts.tile_char);

		int print_width = max(opts.min_width, getWidth());
		int print_height = max(opts.min_height, getHeight());

		// List of rows, top to bottom
		vector<vector<string>> out;

		for (int i = 0; i < getHeight(); ++i) {
			int row = print_height - i - 1;
			int cnt = rows[row];

			vector<string> out_row;

			for (int j = 0; j < cnt; ++j) {
				out_row.push_back(filled_tile);
			}

			for (int j = cnt; j < print_width; ++j) {
				out_row.push_back(empty_tile);
			}

			out.push_back(out_row);
		}

		// Print it out
		std::stringstream ss;

		for (auto row : out) {
			// For each row...
			for (int h = 0; h < tile_height; ++h) {
				// Print the hth character row
				for (auto tile : row) {
					string segment;
					int start_index = h * tile_width;

					if (start_index <= tile.length()) {
						segment = tile.substr(start_index, tile_width);
					}

					ss << setw(tile_width) << segment << setw(0);
					ss << string(horizontal_sep, ' ');
				}

				ss << '\n';
			}

			ss << string(vertical_sep, '\n');
		}

		return ss.str();
	}

	template <int max_height>
	std::ostream& operator<<(std::ostream& os, const Position<max_height>& p) {
		return os << p.to_string();
	}

	/**
	 * Get all positions with exactly n tiles, bounded by bound_width and bound_height (their dimensions are within those
	 * bounds)
	 * @tparam max_height
	 * @tparam Lambda Type of callback function
	 * @param n Number of tiles
	 * @param callback Callback function accepting a single parameter, the position (as a const ref)
	 * @param bound_width -1 if unbounded; otherwise, the bound on the width
	 * @param bound_height -1 if unbounded; otherwise, the bound on the height, superseded by max_height if necessary
	 */
	template <int max_height, typename Lambda>
	void get_positions_with_n_tiles(int n, Lambda callback, int bound_width=-1, int bound_height=-1) {
		static_assert(std::is_invocable_v<Lambda, const Position<max_height>&>,
						"Second parameter to get_positions_with_n_tiles must be a function that accepts Positions");

		if (bound_height == -1) bound_height = INT_MAX;
		if (bound_width == -1) bound_width = INT_MAX;

		// prevent OOB
		bound_height = std::min(bound_height, max_height);

		// We start off with some position and modify it iteratively. We want the rows to sum to n, the number of rows to be
		// less than or equal to the bound_height, and each row to be less than the bound_width. If we are determining the
		// value of a given row r (indexing from 0 as usual), then if we give it a value of v, the maximum number of remaining
		// tiles to place is (bound_height - r) * v and the minimum number is 0. If we have a certain number of remaining
		// tiles to place, these bounds give us information on how many we should try.

		Position<max_height> p; // position to manipulate and be passed to the lambda
		int* rows = p.rows;

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
			int max_place = std::min((i == 0) ? remaining : rows[i-1], bound_width);

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

	namespace {
		}
}

#endif //CHOMP_POSITION_H
