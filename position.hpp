#pragma once

#include <iostream>

#include <array>
#include <algorithm>
#include <initializer_list>
#include <stdexcept>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include "utils.hpp"

#define FILE_LINE CHOMP_FILE_LINE
#define DEBUG CHOMP_DEBUG_VARS
#define DEBUG_NB CHOMP_DEBUG_VARS_NO_BRACES

namespace Chomp {
	// Orientation of a position relative to the "canonical" position, which is flipped about the main diagonal. The
	// canonical position can be thought of as having the lowest "center of mass". For example:
	//     #              #              ##
	//     ##             ###            ##
	//     ###            ###            ###
	// Symmetrical     Canonical     Not canonical
	// Every position has a canonical form; symmetrical positions are always canonical. The multiplicity of a position is
	// 1 when symmetrical and 2 when not symmetrical. Generally the user treats a Position as immutable, though there
	// are a few convenience functions that mutate the position in-place
	enum class Orientation {
		CANONICAL = 0,
		NOT_CANONICAL = 1,
		SYMMETRIC = 2,
		UNKNOWN = 3 // uncalculated
	};

	namespace {
		using O = Orientation;

		bool is_orientation_calculated(O o) noexcept {
			return o != O::UNKNOWN;
		}

		// Silently fails when passed UNKNOWN
		bool is_orientation_canonical(O o) noexcept {
			return !(static_cast<int>(o) & 1);
		}

		// 2 when asymmetric, 1 when symmetric. Silently fails when passed UNKNOWN
		int orientation_multiplicity(Orientation o) noexcept {
			return 2 - (o == O::SYMMETRIC);
		}

		// Convenience alias for converting integer to string (wishing I had std::format!!)
		const auto& str = static_cast<std::string(*)(int)>(std::to_string);
	}
	
	// Represents an arbitrary position with height up to MAX_HEIGHT. We store a position as a list of rows and a height
	// variable. The empty position has a height of 0.
	template<int MAX_HEIGHT>
	class Position {
		static_assert(1 <= MAX_HEIGHT && MAX_HEIGHT <= 1000,
		              "MAX_HEIGHT should be between 1 and 1000. To analyze boards like 3 by 2000, use a lower max height.");

		using rows_type = std::array<int, MAX_HEIGHT>;

	public:
		Position() = default;
		~Position() = default;

		// We only need to copy the first "height" rows
		Position(const Position& p) {
			_height = p._height, _orientation = p._orientation, _square_count = p._square_count;
			std::copy_n(std::begin(p._rows), _height + 1, std::begin(_rows));
		}

		Position& operator=(const Position& p) {
			_height = p._height, _orientation = p._orientation, _square_count = p._square_count;
			std::copy_n(std::begin(p._rows), _height + 1, std::begin(_rows));

			return *this;
		}

		// Convenience constructor; for example, Position{2, 2, 1} gives
		// #
		// ##
		// ##
		Position(std::initializer_list<int> rows) {
			if (rows.size() > MAX_HEIGHT) throw std::runtime_error(FILE_LINE + "Initializer list is too long; "
				+ DEBUG_NB(MAX_HEIGHT) + ", list has size " + str(rows.size()));

			// Initialize rows
			std::copy(std::begin(rows), std::end(rows), std::begin(_rows));

			normalize_height(rows.size());
			if (!is_legal()) throw std::runtime_error(FILE_LINE + "Invalid initializer list: " + list());
			_invalidate_cached(); // ensure square_count and orientation are correct
		}

		// Accessors (note that orientation and square_count values are cached)
		int height() const noexcept { return _height; }
		int width() const noexcept { return _rows[0]; }
		rows_type rows() const {
			rows_type ret{}; // zero-initialized
			std::copy(_rows, _rows + _height, ret);
			return ret;
		}

		// Orientation and square_count will be cached
		Orientation orientation() {
			if (orientation_calculated()) return _orientation;
			return _orientation = _compute_orientation();
		}
		int square_count() {
			if (square_count_calculated()) return _square_count;
			return _square_count = _compute_square_count();
		}

		// Is the position canonical (including symmetric), etc.
		bool is_canonical() { return is_orientation_canonical(orientation()); }
		bool is_symmetric() { return orientation() == O::SYMMETRIC; }
		bool is_empty() const noexcept { return _height == 0; }

		// 0-indexed
		bool square_at(int row, int col) const {
			if (row >= _height || row < 0) return false;
			return _rows[row] >= col;
		}

		// Get the empty position
		static Position empty_position() {
			Position p;
			p._height = p._rows[0] = 0; // rows[0] is set to 0 so that width is valid
			return p;
		}

		// Create a rectangle with the given width and height
		static Position starting_rectangle(int width, int height) {
			if (width < 0 || height < 0)
				throw std::runtime_error(FILE_LINE + "Rectangle must have nonnegative width and height, not " + DEBUG(width, height));

			if (height > MAX_HEIGHT)
				throw std::runtime_error(FILE_LINE + "Rectangle must have a height less than " + DEBUG_NB(MAX_HEIGHT)
					+ ", not " + DEBUG_NB(height));

			Position p;
			p._height = height;
			std::fill_n(std::begin(p._rows), height, width);
			p.orientation = (width < height) ? O::NOT_CANONICAL : ((width == height) ? O::SYMMETRIC : O::CANONICAL);

			return p;
		}

		// Set the height to the appropriate value based on the row values (last non-zero element), with a maximum value of
		// height_bound
		void normalize_height(int height_bound=MAX_HEIGHT) {
			height_bound = std::min(MAX_HEIGHT, height_bound);

			// find first zero element
			for (_height = 0; _height < height_bound && _rows[_height]; ++_height);
		}

		// Is the current position a legal chomp state; note that it ignores row values beyond height
		bool is_legal() const {
			int prev = INT_MAX, curr;

			for (int i = 0; i < _height; ++i) {
				curr = _rows[i];
				if (curr > prev) return false;
				prev = curr;
			}

			return true;
		}

		// Return the position as a readable list of row values
		std::string list() const {
			std::stringstream ss;
			for (int i = 0; i < _height; ++i) {
				ss << _rows[i];
				if (i != _height - 1)
					ss << ' ';
			}
			return ss.str();
		}

		bool orientation_calculated() { return is_orientation_calculated(_orientation); }
		bool square_count_calculated() { return _square_count != -1; }

		template <typename Lambda>
		static bool positions_with_tiles (int min, int max, Lambda callback, int bound_width=-1, int bound_height=-1, bool only_canonical=false) {
			using callback_result = std::invoke_result_t<Lambda, Position&>;
			constexpr bool callback_returns_bool = std::is_same_v<callback_result, bool>;

			for (int i = min; i <= max; ++i) {
				if constexpr (callback_returns_bool) {
					if (positions_with_n_tiles(i, callback, bound_width, bound_height, only_canonical))
						return true;
				} else {
					positions_with_n_tiles(i, callback, bound_width, bound_height, only_canonical);
				}
			}

			return false;
		}

		// Get all positions with n tiles, with a given max width and height (-1 if unspecified). Positions are sent to a
		// callback function, which must accept Position& p. Throws if it should logically return results outside the
		// range of MAX_HEIGHT. Results false if the function was called w/ all positions, and true otherwise. The callback
		// function may return a boolean value; if true, the function will be terminated early. If only_canonical is set to
		// true, only canonical (including symmetrical) positions are calculated and returned, which is roughly half the
		// number of total positions
		template <typename Lambda>
		static bool positions_with_n_tiles (int n, Lambda callback, int bound_width=-1, int bound_height=-1, bool only_canonical=false) {
			static_assert(std::is_invocable_v<Lambda, Position&>,
			        "Callback function must be invocable with a single parameter Position&.");

			using callback_result = std::invoke_result_t<Lambda, Position&>;
			constexpr bool callback_returns_void = std::is_void_v<callback_result>;
			constexpr bool callback_returns_bool = std::is_same_v<callback_result, bool>;

			static_assert(callback_returns_void || callback_returns_bool,
			        "Callback function must either be void or return a boolean");

			if (n < 0) throw std::runtime_error(FILE_LINE + "n must be a positive integer, not " + DEBUG(n));
			if (bound_width < -1 || bound_height < -1)
				throw std::runtime_error(
								FILE_LINE + "bound_width and bound_height must be positive integers or -1, not " + DEBUG(bound_width, bound_height));
			if (n == 0 || bound_width == 0 || bound_height == 0) {
				Position p = empty_position();
				callback(p); // expects an l value

				return false;
			}

			if (bound_width == -1) bound_width = n;
			if (bound_height == -1) bound_height = n;

			// Now guaranteed that n, bound_width, bound_height > 0

			// Logical bounds
			int logical_bound_height = n, logical_bound_width = n;
			if (only_canonical) {
				// The tallest canonical position is an L. If n is odd, it has width and height (n-1)/2; if n is even, it has
				// height n/2 - 1. For example:
				//  #          #
				//  #          #
				//  ###        ####
				// n = 5       n = 6

				logical_bound_height = (n % 2 == 0) ? (n / 2 - 1) : ((n - 1) / 2);
			}

			// Actual bound width and bound height to be used
			bound_width = std::min(logical_bound_width, bound_width);
			bound_height = std::min(logical_bound_height, bound_height);

			if (bound_height > MAX_HEIGHT)
				throw std::runtime_error(
								FILE_LINE + "Call to positions_with_n_tiles will generate positions taller than "
								+ DEBUG_NB(MAX_HEIGHT) + "; " + DEBUG(n, bound_width, bound_height, only_canonical));

			// We start off with some position and modify it iteratively. We want the rows to sum to n, the number of rows to be
			// less than or equal to the bound_height, and each row to be less than the bound_width. If we are determining the
			// value of a given row r (indexing from 0 as usual), then if we give it a value of v, the maximum number of remaining
			// tiles to place is (bound_height - r) * v and the minimum number is 0. If we have a certain number of remaining
			// tiles to place, these bounds give us information on how many we should try.

			Position p = empty_position(); // position to manipulate in place and be passed to the lambda

			// we require the rows array to be all 0s (the constructor doesn't initialize it)
			rows_type& rows = p._rows;
			rows.fill(0);

			// Thinnest canonical position is an L, so n /2
			if (only_canonical) rows[0] = n / 2;

			// Index we are currently modifying
			int i = 0;
			// Tiles remaining to be placed
			int remaining = n;

			// Cache min_place and max_place
			rows_type min_place_arr;
			rows_type max_place_arr;

			// Whether min_place and max_place need to be recalculated
			bool needs_recalc = true;

			while (true) {
				int rows_remaining = bound_height - i;

				int current = rows[i];
				remaining += current;

				int min_place, max_place;
				if (needs_recalc) {
					// ceiling division of remaining and rows_remaining; we need to place at least this many squares
					min_place = min_place_arr[i] = (remaining + rows_remaining - 1) / rows_remaining;
					// Upper bound on how many squares to place
					max_place = max_place_arr[i] = std::min((i == 0) ? remaining : rows[i - 1], bound_width);
					needs_recalc = false;
				} else {
					min_place = min_place_arr[i];
					max_place = max_place_arr[i];
				}

				if (current >= max_place || max_place < min_place) {
					// backtrack
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
					p._height = (current == 0) ? i : (i + 1);

					if (!only_canonical || p.is_canonical()) {
						if constexpr (callback_returns_bool) {
							if (callback(p)) return true;
						} else {
							callback(p);
						}
					}

					p._invalidate_cached();

					rows[i--] = 0;
					remaining += current;

					if (i == -1) break;
				} else {
					// Some tiles remain to be placed; continue if possible
					i = std::min(i + 1, bound_height - 1);
					needs_recalc = true;
				}
			}

			return false;
		}

	public:
		rows_type _rows;
		int _height;

		// We cache the orientation and square count
		Orientation _orientation;
		int _square_count;

		Orientation _compute_orientation() {
			// Easy and common criteria
			if (_rows[0] > _height) return O::CANONICAL;
			if (_rows[0] < _height) return O::NOT_CANONICAL;
			if (_height == 0) return O::SYMMETRIC;

			// We calculate the number of tiles in each column and compare sequentially to rows[col]
			int col = 1, min_height = 0;
			for (int i = _height - 1; i >= min_height; --i) {
				int r = _rows[i];
				while (r > col) {
					// i is the number of tiles in column col
					// compare col and row_compare
					if (i + 1 > _rows[col])
						return O::NOT_CANONICAL;
					else if (i + 1 < _rows[col])
						return O::CANONICAL;

					col++;
				}
			}

			return O::SYMMETRIC; // if all are equal, symmetrical
		}

		int _compute_square_count() {
			int sum = 0;
			for (int i = 0; i < _height; ++i)
				sum += _rows[i];
			return sum;
		}

		// Internal functions used when manipulating Positions in place
		void _invalidate_cached () { _orientation = O::UNKNOWN; _square_count = -1; }
		void _make_internally_empty() { std::fill(_rows.begin(), _rows.end(), 0); }
	};

	// Let p(n) be the partition function. Let p(N, M; n) be the number of positions with n tiles in an MxN rectangle.
	// Clearly p(n) >= p(N, M; n) and if N,M <= n, p(n) = p(N, M; n). p(N,M;n) = p(N,M-1;n)+p(N-1,M,n-M), and
	// p(N,M;n)=p(M,N;n).

	using p_count_type = uint64_t;

	p_count_type partition_function(int n);
	// A value of -1 in any of the last three positions indicates it should be unbounded
	p_count_type count_positions(int min_squares=0, int max_squares=-1, int bound_width=-1, int bound_height=-1);
}

#undef FILE_LINE
#undef DEBUG
#undef DEBUG_NB