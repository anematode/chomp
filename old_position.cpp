#include <old_position.hpp>
#include <store.hpp>
#include <datastructs.hpp>
#include <unordered_map>
#include <thread>
#include <memory>
#include <atomic>

#undef INT_MAX
#define INT_MAX 2147483647

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

	Orientation Position::is_canonical() {
		using O = Orientation;

		if (o == O::UNKNOWN) o = _is_canonical();

		return o;
	}

	void Position::make_canonical() {
		Orientation canonical = is_canonical();

		if (canonical == Orientation::CANONICAL || canonical == Orientation::SYMMETRICAL) return;
		// Time to flip
		flip();
	}

	void Position::flip_in_place() {
		// Flip the position across the diagonal. Note that if rows[0] >= MAX_HEIGHT it may be chopped off
		int col = 0;
		int new_rows[MAX_HEIGHT];

		for (int i = height - 1; i >= 0; --i) {
			int row = rows[i];

			while (row > col) {
				new_rows[col] = i + 1;
				col++;

				if (col == MAX_HEIGHT) goto done;
			}
		}
		done:

		height = col;
		std::copy(new_rows, new_rows+height, rows);

		using O = Orientation;
		if (o == O::NOT_CANONICAL)
			o = O::CANONICAL;
		else if (o == O::CANONICAL)
			o = O::NOT_CANONICAL;
	}

	Position Position::flip() {
		Position p = *this;

		p.flip_in_place();

		return p;
	}

	Orientation Position::_is_canonical() const {
		using O = Orientation;

		// Easy criteria
		if (rows[0] > height) return O::CANONICAL;
		if (height == 0) return O::SYMMETRICAL;
		if (rows[0] < height) return O::NOT_CANONICAL;

		// We calculate the number of tiles in each column and compare sequentially to rows[col]
		int col = 1, min_height = 0;
		for (int i = height - 1; i >= min_height; --i) {
			int r = rows[i];
			while (r > col) {
				// i is the number of tiles in column col
				// compare col and row_compare
				if (i + 1 > rows[col])
					return O::NOT_CANONICAL;
				else if (i + 1 < rows[col])
					return O::CANONICAL;

				col++;
			}
		}

		return O::SYMMETRICAL; // symmetrical
	}

	Position Position::canonical() const {
		Position p = *this;
		p.make_canonical();

		return p;
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

	map_type losing_position_info;
	Chomp::datastructs::BloomFilter bloom_losing_position_info;

	PositionInfo Position::info() const {
		if (height == 0) return { .is_winning=true, .dte=0 };

		uint64_t canonical_hash = this->canonical_hash();

		bool definitely_contains = bloom_losing_position_info.probably_contains(canonical_hash);

		if (definitely_contains) {
			auto losing_position = losing_position_info.find(canonical_hash);

			if (losing_position != losing_position_info.end()) return { .is_winning=false, .dte=losing_position->second.dte };
		}

		// Winning position
		int min_dte = INT_MAX;

		for_each_cut([&] (Cut c) {
			Position cutted = cut(c);
			canonical_hash = cutted.canonical_hash();
			if (bloom_losing_position_info.probably_contains(canonical_hash)) {
				auto cutted_info = losing_position_info.find(canonical_hash);

				if (cutted_info != losing_position_info.end()) {
					// For all losing cuts
					int dte = cutted_info->second.dte;
					min_dte = std::min(dte+1, min_dte);
				}
			}
		});

		return { .is_winning=true, .dte=min_dte };
	}

	std::atomic<int> num_positions;
	std::atomic<int> num_winning_moves;
	std::atomic<int> num_losing_positions;

	using position_iterator = std::vector<Position>::iterator;
	void hash_positions_over_iterator(map_type& map, std::vector<uint64_t>& bloomqueue, position_iterator begin, position_iterator end, HashPositionOptions opts={}) {
		std::vector<Position> cutted_list;

		for (auto it = begin; it != end; ++it) {
			Position p = *it;

			bool is_winning = false;
			int multiplicity = (p.o == Orientation::CANONICAL) ? 2 : 1;

			num_positions += multiplicity;

			p.for_each_cut([&] (Cut c) {
				Position cutted = p.cut(c);
				
				if (opts.compute_dte) cutted_list.push_back(cutted);

				uint64_t canonical_hash = cutted.canonical_hash();

				if (bloom_losing_position_info.probably_contains(canonical_hash)) {
					auto cutted_info = losing_position_info.find(canonical_hash);

					if (cutted_info != losing_position_info.end()) {
						is_winning = true;
						num_winning_moves += multiplicity;
					}
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
				uint64_t h = p.canonical_hash();
				map[h] = { .dte = max_dte };
				bloomqueue.push_back(h);
				// bloom_losing_position_info.query(h);
				num_losing_positions += multiplicity;
			}

			cutted_list.clear();
		}
	}

	void hash_positions(int max_squares, int bound_width, int bound_height, HashPositionOptions opts) {
		const unsigned POSITION_BATCH_SIZE = 1000000; // how many positions to process at once
		const int NUM_THREADS = 8;

		std::vector<Position> positions;

		auto process_positions = [&] {
			size_t size = positions.size();

			if (size > 10000) {
				std::vector<std::thread> threads;

				// Divvy up the work, with size/NUM_THREADS each
				int positions_per_thread = size / NUM_THREADS;

				position_iterator begin = positions.begin();
				//map_type map1, map2, map3, map4;
				std::vector<map_type*> maps;
				std::vector<std::vector<uint64_t>*> bloomqueues;

				for (int i = 0; i < NUM_THREADS; ++i) {
					position_iterator end = (i == NUM_THREADS - 1) ? positions.end() : (begin + positions_per_thread);
					map_type* thread_map = new map_type{};
					std::vector<uint64_t>* thread_bloomqueue;

					maps.push_back(thread_map);
					bloomqueues.push_back(thread_bloomqueue);

					std::thread thread ([=] (map_type* map, std::vector<uint64_t>* bloomqueue) {
						hash_positions_over_iterator(*map, *bloomqueue, begin, end);
					}, maps.back(), bloomqueues.back());

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

				for (std::vector<uint64_t>* bloomqueue : bloomqueues) {
					for (uint64_t hash : *bloomqueue) {
						bloom_losing_position_info.insert(hash);
					}

					delete bloomqueue;
				}
			} else {
				std::vector<uint64_t> bloomqueue;
				
				hash_positions_over_iterator(losing_position_info, bloomqueue, positions.begin(), positions.end(), opts);

				for (uint64_t hash : bloomqueue) {
					bloom_losing_position_info.insert(hash);
				}
			}
		};

		for (int n = 1; n <= max_squares; ++n) {
			num_winning_moves = num_positions = num_losing_positions = 0;

			get_positions_with_n_tiles(n, [&] (const Position& p) {
				positions.push_back(p);

				// Process positions in batches
				if (positions.size() > POSITION_BATCH_SIZE) {
					process_positions();
					positions.clear();
				}
			}, bound_width, bound_height, true /* only canonical positions */);

			// Process remaining positions
			process_positions();
			positions.clear();

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

	// Return the hash of the flipped position
	uint64_t hash_flipped_position(const Position &p) {
		int col = 0;
		uint64_t hash = 0;

		for (int i = p.height - 1; i >= 0; --i) {
			int row = p.rows[i];

			while (row > col) {
				hash += i + 1;
				hash *= 179424673L;
				col++;
			}
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
	  o = p.o;
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

	uint64_t Position::canonical_hash() const {
		if (o == Orientation::CANONICAL || o == Orientation::SYMMETRICAL) return hash_position(*this);
		if (o == Orientation::NOT_CANONICAL || (_is_canonical() == Orientation::NOT_CANONICAL)) return hash_flipped_position(*this);
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

	void store_positions(const std::string& filename) {
		store_positions(filename.c_str());
	}

	void store_positions(const char* filename) {
		store::write_map(losing_position_info, filename);
	}

	void load_positions(const std::string& filename) {
		load_positions(filename.c_str());
	}

	void load_positions(const char* filename) {
		store::read_map(losing_position_info, filename);
	}
}