#include <position.h>
#include <unordered_map>
#include <thread>
#include <memory>

#define INT_MAX std::numeric_limits<int>::max()

namespace Chomp {
	std::ostream &operator<<(std::ostream &os, const Position &p) {
		return os << p.to_string();
	}

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

	Position Position::cut(int row, int col) const {
		Position p;

		for (int i = 0; i < row; ++i) {
			p.rows[i] = rows[i];
		}

		if (col == 0) {
			p.height = row;
		} else {
			for (int i = row; i < height; ++i) {
				p.rows[i] = std::min(col, rows[i]);
			}

			p.height = height;
		}

		return p;
	}

	Position Position::cut(Cut c) const {
		return cut(c.first, c.second);
	}

	using map_type = std::unordered_map<uint64_t, LosingPositionInfo>;
	map_type losing_position_info;

	void Position::reflect_if_necessary() {
		if (height == 0) return;

		int cols[MAX_HEIGHT];

		int ri = height - 1;
		int ci = 0;

		while (ri >= 0) {
			if (ci >= rows[ri]) {
				ri--;
				continue;
			}
			cols[ci++] = ri + 1;
		}

		int i = 0;
		do {
			if (cols[i] > rows[i]) {
				std::copy(cols, cols+MAX_HEIGHT, rows);
				height = ci;
				return;
			}
		} while(rows[i] == cols[i++]);
	}

	PositionInfo Position::info() const {
		if (height == 0) return { .is_winning=true, .dte=0 };

		auto losing_position = losing_position_info.find(hash());
		if (losing_position == losing_position_info.end()) {
			// Winning position
			int min_dte = INT_MAX;

			for_each_cut([&] (Cut c) {
				Position cutted = cut(c);
				auto cutted_info = losing_position_info.find(cutted.hash());

				if (cutted_info != losing_position_info.end()) {
					// For all losing cuts
					int dte = cutted_info->second.dte;
					min_dte = std::min(dte+1, min_dte);
				}
			});

			return { .is_winning=true, .dte=min_dte };
		} else {
			return { .is_winning=false, .dte=losing_position->second.dte };
		}
	}

	std::atomic<int> num_positions;
	std::atomic<int> num_winning_moves;
	std::atomic<int> num_losing_positions;

	using position_iterator = std::vector<Position>::iterator;
	void hash_positions_over_iterator(map_type& map, position_iterator begin, position_iterator end, HashPositionOptions opts={}) {
		std::vector<Position> cutted_list;

		for (auto it = begin; it != end; ++it) {
			Position p = *it;

			bool is_winning = false;

			num_positions++;

			p.for_each_cut([&] (Cut c) {
				Position cutted = p.cut(c);
				auto cutted_info = losing_position_info.find(cutted.hash());

				if (opts.compute_dte) cutted_list.push_back(cutted);

				if (cutted_info != losing_position_info.end()) {
					is_winning = true;
					num_winning_moves++;
				}
			});

			int max_dte = 0;
			if (!is_winning && opts.compute_dte) {
				for (Position& cutted : cutted_list) {
					int dte = cutted.info().dte;

					max_dte = std::max(dte+1, max_dte);
				}
			}

			if (!is_winning) {
				map[p.hash()] = { .dte = max_dte };
				num_losing_positions++;
			}

			cutted_list.clear();
		}
	}

	void hash_positions(int max_squares, int bound_width, int bound_height, HashPositionOptions opts) {
		for (int n = 1; n <= max_squares; ++n) {
			std::vector<Position> positions;
			get_positions_with_n_tiles(n, [&] (const Position& p) {
				positions.push_back(p);
			}, bound_width, bound_height);

			num_winning_moves = num_positions = num_losing_positions = 0;

			size_t size = positions.size();
			if (size > 10000) {
				const int NUM_THREADS = 8;
				std::vector<std::thread> threads;

				// Divvy up the work, with size/NUM_THREADS each
				int positions_per_thread = size / NUM_THREADS;

				position_iterator begin = positions.begin();
				//map_type map1, map2, map3, map4;
				std::vector<map_type*> maps;

				for (int i = 0; i < NUM_THREADS; ++i) {
					position_iterator end = (i == NUM_THREADS - 1) ? positions.end() : (begin + positions_per_thread);
					maps.push_back(new map_type{});

					std::thread thread ([=] (map_type* map) {
						hash_positions_over_iterator(*map, begin, end);
					}, maps.back());

					threads.push_back(std::move(thread));

					begin = end;
				}

				for (auto &thread : threads) {
					thread.join();
				}

				for (map_type* map : maps) {
					losing_position_info.merge(*map);
					delete map;
				}
			} else {
				hash_positions_over_iterator(losing_position_info, positions.begin(), positions.end(), opts);
			}

			//std::printf("%i\t%f\n", n, num_winning_moves / (float) num_positions);
			std::printf("%i %i %i %i\n", n, (int)num_positions, (int)num_winning_moves, (int)num_losing_positions);
		}
	}

	std::vector<Cut> Position::winning_cuts() const {
		std::vector<Cut> ret;

		for_each_cut([&] (Cut c) {
			Position cutted = cut(c);
			if (!cutted.info().is_winning)
				ret.push_back(c);
		});

		return ret;
	}

	int Position::num_winning_cuts() const {
		int ret = 0;

		for_each_cut([&] (Cut c) {
			Position cutted = cut(c);
			if (!cutted.info().is_winning)
				ret++;
		});

		return ret;
	}


	uint64_t hash_position(const Position &p) {
		// It isn't the greatest hash function, but it works. The hash function *is* dependent on MAX_HEIGHT
		const int *rows = p.rows;

		uint64_t hash = 0;
		for (int i = 0; i < p.height; ++i) {
			hash += rows[i];
			hash *= 179424673L;
		}

		return hash;
	}

	Position::Position() {

	}

	void Position::make_empty() {
		height = 0;
		for (int i = 0; i < MAX_HEIGHT; ++i) {
			rows[i] = 0;
		}
	}

// copy constructor
	Position::Position(const Position &p) {
		height = p.height;
		for (int i = 0; i < height; ++i) {
			rows[i] = p.rows[i];
		}
	}

	Position Position::starting_rectangle(int width, int height) {
		Position p;

		if (width < 0) throw new std::runtime_error(FILE_LINE
		"Width cannot be negative");

		std::fill_n(p.rows, std::min(height, MAX_HEIGHT), width);
		p.height = height;

		return p;
	}

	Position Position::empty_position() {
		Position p;
		p.make_empty();
		return p;
	}

	uint64_t Position::hash() const {
		return hash_position(*this);
	}

	int Position::square_count() const {
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
	}

	Position::Position(const std::initializer_list<int> &l) {
		int h = std::min(MAX_HEIGHT, static_cast<int>(l.size()));

		// Is there a nicer way to do this?
		std::copy(l.begin(), l.begin() + h, rows);

		for (int i = h; i < MAX_HEIGHT; ++i) rows[i] = 0;
		height = h;

		if (!is_legal()) throw std::runtime_error(FILE_LINE"Invalid position in initializer list");
		normalize_height();
	}

	void Position::normalize_height() {
		// Set the height value to the appropriate value based on the rows array
		int i = 0;
		for (; i < MAX_HEIGHT && rows[i]; ++i);

		height = i;
	}

	int Position::get_height() const {
		return height;
	}

	int Position::get_width() const {
		return rows[0];
	}

	std::string Position::list() const {
		std::stringstream ss;

		for (int i = 0; i < height; ++i)
			ss << rows[i] << ' ';

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

		for (int i = 0; i < print_height; ++i) {
			vector<string> out_row;

			int cnt = 0;

			if (i < height) {
				cnt = rows[i];

				for (int j = 0; j < cnt; ++j) {
					out_row.push_back(filled_tile);
				}
			}

			for (int j = cnt; j < print_width; ++j) {
				out_row.push_back(empty_tile);
			}

			out.insert(out.begin(), out_row);
		}

		// Add row/col markers if desired
		if (opts.show_labels) {
			for (int row = 0; row < print_height; ++row) {
				out[row].insert(out[row].begin(), get_row_marker(print_height - row - 1)); // invert the labels
			}

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
}