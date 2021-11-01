//
// Created by Timothy Herchen on 10/31/21.
//

#ifndef CHOMP_POSITION_H
#define CHOMP_POSITION_H

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

// Get the file and line number
#define FILE_LINE __FILE__ ":" STR(__LINE__) ": "

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
	constexpr int MAX_HEIGHT = 5;

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

	PositionFormatterOptions default_formatter_options;

	struct PositionInfo {
		bool is_winning; // whether the position is winning
		int dte; // distance to game end, assuming optimal play
	};

	using Cut = std::pair<int, int>;

	/**
	 * Stores a given board position as an array of the number of tiles in each row, from bottom to top
	 * @tparam max_height The tallest allowed board
	 */
	class Position {
	public:
		int rows[MAX_HEIGHT];
		int cols[MAX_HEIGHT];

		/**
		 * #X
		 * ##X
		 * ####X
		 *
		 * rows: 0 1 2
		 * cols: 4 2 1
		 * corner_count: 3
		 */

		int corner_count; // The number of corners

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

		static Position starting_rectangle(int width, int height);
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

	void PositionFormatterOptions::set_default(PositionFormatterOptions opts) {
		default_formatter_options = opts;
	}

	//
	// #
	// ##
	// ##XXX
	// #####
	// ######

	// Create a corner, copy a corner, remove a corner
	// Case A:
	// If a corner (r, c) is within (row, col), i.e. r >= row and c >= col, we remove it
	// Case B:
	// If a corner (r, c) is below row-1 or to the left of c-1, we copy it
	// Case C:
	// Remaining cases: r=row and c=col, r=row and c < col, r=row and c > col, r>row and c=col, r<row and c=col
	// Case 1: r=row and c=col
	// Create corner at (row-1,c) if there is no corner with that row, and at (r,col+1) if there is no corner with that col
	// Case 2: r=row and c<col
	// Create corner at (row-1,c) if there is no corner with that row
	// Case 3: r=row and c>col
	// Cut is out of bounds; do nothing
	// Case 4: r>row and c=col
	// Cut is out of bounds; do nothing
	// Case 5: r<row and c=col
	// Create corner at (r,col+1) if there is no corner with that col

	Position Position::cut(int row, int col) const {
		Position p;

		int write_index = 0;
		int cr, cc, i = 0;

		for (; i < corner_count; ++i) {
			cr = rows[i];
			cc = cols[i];

			if (cr < row) {
				// Copy it (Case A)
				p.rows[write_index] = cr;
				p.cols[write_index] = cc;
				write_index++;
			} else {
				break;
			}
		}

		// Now cr == row or cr > row
		if (cc >= col) {
			// There is something to cut. Find first and last corners within the zone
			int first_corner = i;

			for (; i < corner_count && cols[i] >= col; ++i);
			int last_corner = i - 1;

			// First corner determines whether we put a new corner at (row-1, cols[first_corner])
			// We do so if there are no squares to the right of the new corner; in other words, whether rows[first_corner-1] == row-1
			// Last corner determines whether we put a new corner at (rows[last_corner], col-1)
			// We do so if there are no squares above the new corner; in other words, whether cols[last_corner+1] == col-1
			// If row-1 or col-1 is -1, we don't put a corner
			// If any of these queries are out of bounds, we say there are squares there if the other value in the pair is not 0

			// First corner
			if ((row != 0) && ((first_corner == 0) || rows[first_corner - 1] != row - 1)) {
				// need to add corner
				p.rows[write_index] = row-1;
				p.cols[write_index] = cols[first_corner];

				write_index++;
			}

			if ((col != 0) && ((last_corner == corner_count - 1) || cols[last_corner + 1] != col - 1)) {
				// need to add corner
				p.rows[write_index] = rows[last_corner];
				p.cols[write_index] = col-1;

				write_index++;
			}
		}

		// Copy the rest
		for (; i < corner_count; ++i) {
			cr = rows[i];
			cc = cols[i];

			p.rows[write_index] = cr;
			p.cols[write_index] = cc;
			write_index++;
		}

		p.corner_count = write_index;

		return p;
	}

	Position Position::cut (Cut c) const {
		return cut(c.first, c.second);
	}

	/*template <int max_height>
	void PositionSet<max_height>::hash_positions(int max_squares, int bound_width, int bound_height) {
		Position p;
		p.make_empty();

		// Empty position is winning
		PositionInfo empty_position = { .is_winning = true, .dte = 0 };
		position_info[p.hash()] = empty_position;


	}*/

	uint64_t hash_position(const Position& p) {
		// It isn't the greatest hash function, but it works. The hash function *is* dependent on max_height
		const int* rows = p.rows;
		const int* cols = p.cols;

		uint64_t hash = 0;
		for (int i = 0; i < MAX_HEIGHT; ++i) {
			hash += rows[i];
			hash *= 179424673L;
			hash += cols[i];
			hash *= 179424673L;
		}

		return hash;
	}

	Position::Position() {

	}

	void Position::make_empty() {
		corner_count = 0;
	}

	// copy constructor
	Position::Position(const Position& p) {
		corner_count = p.corner_count;
		for (int i = 0; i < corner_count; ++i) {
			rows[i] = p.rows[i];
			cols[i] = p.cols[i];
		}
	}

	Position Position::starting_rectangle(int width, int height) {
		Position p;

		p.corner_count = 1;
		p.rows[0] = height - 1;
		p.cols[0] = width - 1;

		return p;
	}

	uint64_t Position::hash() const {
		return hash_position(*this);
	}

	/*int Position::square_count() const {
		int sum = 0;
		for (int i = 0; i < height; ++i)
			sum += rows[i];
		return sum;
	}

	bool Position::is_legal() const {
		int prev = INT32_MAX; // :)

		for (int i = 0; i < height; ++i) {
			int cnt = rows[i];
			if (cnt > prev || cnt < 0)
				return false;
			prev = cnt;
		}

		return true;
	}*/

	/*Position::Position(const std::initializer_list<int>& l) {
		int h = std::min(max_height, static_cast<int>(l.size()));

		// Is there a nicer way to do this?
		std::copy(l.begin(), l.begin() + h, rows);

		for (int i = h; i < max_height; ++i) rows[i] = 0;

		if (!is_legal()) throw std::runtime_error(FILE_LINE"Invalid position in initializer list");
		normalize_height();
	}*/

	int Position::get_height() const {
		return rows[corner_count - 1] + 1;
	}

	int Position::get_width() const {
		return cols[0] + 1;
	}

	std::string Position::list() const {
		std::stringstream ss;

		ss << "R ";
		for (int i = 0; i < corner_count; ++i)
			ss << rows[i] << ' ';
		ss << "\nC ";
		for (int i = 0; i < corner_count; ++i)
			ss << cols[i] << ' ';

		return ss.str();
	}

	// TODO: make work for labels greater than 9
	std::string Position::to_string(PositionFormatterOptions opts) const {
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

		int print_width = max(opts.min_width, get_width());
		int print_height = max(opts.min_height, get_height());

		// List of rows, top to bottom
		vector<vector<string>> out;

		// Get an appropriate string for a left marker on a given row
		auto get_row_marker = [&] (int r) {
			string marker = string(tile_area, ' ');
			string as_str = std::to_string(r);

			// Center it vertically and right-align it
			marker.insert((tile_area + 1) / 2 + tile_width - as_str.length() - 1, as_str);
			return marker;
		};

		// Get an appropriate string for a top marker on a given column
		auto get_col_marker = [&] (int c) {
			string marker = string(tile_area, ' ');
			string as_str = std::to_string(c);

			// Center it vertically and put it on the bottom
			marker.insert(tile_area - tile_width / 2 - (as_str.length() + 1) / 2, as_str);
			return marker;
		};

		int corner_index = 0;
		int height = get_height();

		for (int i = 0; i < height; ++i) {
			if (corner_index < corner_count && rows[corner_index] < i) {
				corner_index++;
			}

			int cnt = cols[corner_index] + 1;
			vector<string> out_row;

			for (int j = 0; j < cnt; ++j)
				out_row.push_back(filled_tile);
			for (int j = cnt; j < print_width; ++j)
				out_row.push_back(empty_tile);

			out.insert(out.begin(), out_row);
		}

		// Add row/col markers if desired
		if (opts.show_labels) {
			for (int row = 0; row < print_height; ++row)
				out[row].insert(out[row].begin(), get_row_marker(print_height - row - 1)); // invert the labels

			vector<string> first_row; // new first row
			first_row.push_back(string(tile_area, ' ')); // spacer

			for (int col = 0; col < print_width; ++col)
				first_row.push_back(get_col_marker(col));

			out.insert(out.begin(), first_row);
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

	std::ostream& operator<<(std::ostream& os, const Position& p) {
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
	/*template <typename Lambda>
	void get_positions_with_n_tiles(int n, Lambda callback, int bound_width=-1, int bound_height=-1) {
		static_assert(std::is_invocable_v<Lambda, const Position&>,
						FILE_LINE"Second parameter to get_positions_with_n_tiles must be a function that accepts Positions with the same max_height");

		if (n < 0)
			throw new std::runtime_error(FILE_LINE"n must be a positive integer");

		if (bound_height == -1) bound_height = INT_MAX;
		if (bound_width == -1) bound_width = INT_MAX;

		// prevent OOB
		bound_height = std::min(bound_height, MAX_HEIGHT);

		// We start off with some position and modify it iteratively. We want the rows to sum to n, the number of rows to be
		// less than or equal to the bound_height, and each row to be less than the bound_width. If we are determining the
		// value of a given row r (indexing from 0 as usual), then if we give it a value of v, the maximum number of remaining
		// tiles to place is (bound_height - r) * v and the minimum number is 0. If we have a certain number of remaining
		// tiles to place, these bounds give us information on how many we should try.

		Position<max_height> p; // position to manipulate and be passed to the lambda
		p.make_empty();
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
	}*/

	/**
	 * Get all positions with n tiles or less, bounded by bound_width and bound_height (their dimensions are within those
	 * bounds). The order in which the positions are given is guaranteed to be in order of number of squares.
	 * @tparam max_height
	 * @tparam Lambda Type of callback function
	 * @param n Number of tiles
	 * @param callback Callback function accepting a single parameter, the position (as a const ref)
	 * @param bound_width -1 if unbounded; otherwise, the bound on the width
	 * @param bound_height -1 if unbounded; otherwise, the bound on the height, superseded by max_height if necessary
	 */
	/*template <int max_height, typename Lambda>
	void get_positions_with_n_or_less_tiles(int n, Lambda callback, int bound_width=-1, int bound_height=-1) {
		for (int i = 0; i < n; ++i) {
			get_positions_with_n_tiles<max_height>(i, callback, bound_width, bound_height);
		}
	}*/
}

#endif //CHOMP_POSITION_H
