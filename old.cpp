#include <iostream>
#include <cstring>
#include <sstream>
#include <cmath>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <chrono>
#include <optional>
#include <array>

// The dimensions of the game
const int width = 9;
const int height = 9;

const int VERBOSE = 0;

const char tile_character = '#';
const char blank_character = ' ';

/**
 * We represent game positions as an array of integers of size height, where each integer is the number of chocolate pieces still left in that row. For example, [1, 4, 5] represents the following bar:
 _________
 #
 ####
 #####
  
 The bar must be monotonically increasing. Entries can be 0.
 What does a cut look like? Well, we select one of the nonzero rows, then select a number strictly less than that row's value, then limit all the entries before it to the minimum of itself and the row's value.

 For example, take [1, 4, 5] at index 1 and cut = 2. Then it becomes [1, 2, 5]:
 _________
 #
 ##.
 #####

 The dot shows where the cut was made.
 */

struct PositionProperties;
struct GamePosition;

typedef std::pair<int, int> Cut;
typedef std::pair<GamePosition, Cut> Move;

struct GamePosition {
	std::array<int, height> rows;

	GamePosition() {};
	GamePosition(std::array<int, height> rows);
	PositionProperties props() const;

	bool is_winning() const;
	int tile_count() const;

	std::string to_string() const;
	std::string pretty_print(int square_width = 2, int square_height = 3, int square_space_row = 1 /* sponsor me! */, int square_space_col = 2, char tile_char=tile_character, char blank_char=blank_character) const;

	bool is_valid_cut(Cut) const;
	GamePosition cut(Cut) const;

	std::vector<Cut> get_valid_cuts() const;
	std::vector<Move> get_moves() const;

	std::vector<Move> get_winning_moves() const;
	std::vector<Move> get_losing_moves() const;

	std::optional<Cut> get_losing_cut_longest_delay();
	std::optional<Cut> get_losing_cut_trickiest_ratio();
	std::optional<Cut> get_winning_cut_shortest_delay();

	void print_info_about_position(bool show_next_moves=true) const;
	size_t get_hash();

	bool operator== (const GamePosition& p) const;

	static GamePosition starting_position();
};

auto hash_function = [] (const GamePosition& x) {
	size_t hash = x.rows[0];

	for (int i = 1; i < height; ++i) {
		hash *= height;
		hash += x.rows[i];
	}

	return hash;
};

size_t GamePosition::get_hash() {
	return hash_function(*this);
}

std::vector<Move> GamePosition::get_winning_moves() const {
	auto all_moves = get_moves();

	auto res = std::remove_if(all_moves.begin(), all_moves.end(), [&] (auto m) {
		return m.first.is_winning();
	});

	all_moves.erase(res, all_moves.end());

	return all_moves;
}

std::vector<Move> GamePosition::get_losing_moves() const {
	auto all_moves = get_moves();

	auto res = std::remove_if(all_moves.begin(), all_moves.end(), [&] (Move m) {
		return m.first.is_winning();
	});

	all_moves.erase(res, all_moves.end());

	return all_moves;
}

/*std::optional<Cut> get_random_winning_cut() {

}*/

struct PositionProperties {
	bool is_winning; // Whether the position is winning for the player who has it
	int distance_to_end; // What the maximum distance to the end is, assuming optimal play by BOTH sides (how many turns before the end)
	int winning_move_count; // How many moves can you play to win
	int losing_move_count; // How many moves can you play to lose
	int id; // which position it was

	PositionProperties(bool is_winning=false, int distance_to_end=0, int winning_move_count=0, int losing_move_count=0, int id=0) : is_winning(is_winning), distance_to_end(distance_to_end), winning_move_count(winning_move_count), losing_move_count(losing_move_count), id(id) {}
};

/** The map that contains all the position relationship data */
std::unordered_map<GamePosition, PositionProperties, decltype(hash_function)> position_map {1, hash_function};

bool is_valid_position(std::array<int, height> rows) {
	int prev = width;

	for (int i = 0; i < height; ++i) {
		int r = rows[i];
		if (r < 0 || r > width || r > prev) {
			return false;
		}
		prev = r;
	}

	return true;
}

GamePosition::GamePosition(std::array<int, height> rows) {
	if (!is_valid_position(rows)) throw std::invalid_argument("The rows are not in decreasing order, or are out of range.");

	this->rows = rows;
}

bool GamePosition::is_winning() const {
	return props().is_winning;
}

PositionProperties GamePosition::props() const {
	return position_map.at(*this);
}

GamePosition GamePosition::cut(Cut c) const {
	int row = c.first, column = c.second;
	GamePosition ret;

	for (int i = 0; i < row; ++i)
		ret.rows[i] = rows[i];
	for (int i = row; i < height; ++i)
		ret.rows[i] = std::min(rows[i], column);

	return ret;
}

GamePosition GamePosition::starting_position() {
	std::array<int, height> rows;
	rows.fill(width);
	return { rows };
}

std::vector<Cut> GamePosition::get_valid_cuts() const {
	std::vector<Cut> out;

	for (int i = 0; i < height; ++i) {
		int row = rows[i];
		for (int j = 0; j < row; ++j) { // try to cut on any of the squares
			out.push_back({i, j});
		}
	}

	return out;
}

std::vector<Move> GamePosition::get_moves() const {
	std::vector<Move> out;

	auto cuts = get_valid_cuts();
	std::for_each(cuts.begin(), cuts.end(), [&] (Cut cut) { out.push_back({ this->cut(cut), cut }); });

	return out;
}

bool GamePosition::operator==(const GamePosition& p) const {
	return rows == p.rows;
}

bool GamePosition::is_valid_cut(Cut c) const {
	int row = c.first, column = c.second;

	return width > column && column >= 0 && row >= 0 && height > row && rows.at(row) > column;
}

std::string GamePosition::to_string() const {
	std::stringstream ss;

	for (int i = height - 1; i >= 0; --i) {
		int count = rows[i];
		for (int j = 0; j < width; ++j)
			ss << ((j < count) ? tile_character : blank_character);
		ss << '\n';
	}

	return ss.str();
}

enum Condition {
	all,
	winning,
	losing
};


std::string gen_pretty_rowcol_square(int square_height, int square_width, int num, bool is_column=false) {
	using namespace std;

	int area = square_height * square_width;
	string num_str = std::to_string(num);
	int offset;

	if (is_column) {
		offset = area - square_width / 2 - 1;
	} else {
		offset = square_width * (square_height / 2 + 1) - num_str.length();
	}

	return string(offset, ' ') + num_str + string(area - num_str.length() - offset, ' ');
}

std::string GamePosition::pretty_print(int square_height, int square_width, int square_space_row /* sponsor me! */, int square_space_col, char tile_char, char blank_char) const {
	using namespace std;
	// We split up the print into squares of size square_size, *then* arrange them into a single string. The contents of each square are generated as a single string.
	array<array<string, width + 1>, height + 1> output;

	int area = square_width * square_height;
	string sq_template(area, ' ');

	// column 0 is reserved for row numbers, row 0 is reserved for column numbers

	for (int row = 1; row <= height; ++row)
		output[row][0] = gen_pretty_rowcol_square(square_height, square_width, height - row + 1, false);
	for (int col = 1; col <= width; ++col)
		output[0][col] = gen_pretty_rowcol_square(square_height, square_width, col, true);

	for (int row = 1; row <= height; ++row) {
		for (int col = 1; col <= width; ++col) {
			char to_use = (rows[height - row] >= col) ? tile_char : blank_char;
			output[row][col] = string(area, to_use);
		}
	}

	std::stringstream ss;

	for (int row = 0; row <= height; ++row) {
		for (int row_subindex = 0; row_subindex < square_height; ++row_subindex) {
			for (int col = 0; col <= width; ++col) {
				if (output[row][col].length() >= area) {
					ss << output[row][col].substr(row_subindex * square_width, square_width);
				} else {
					ss << string(square_width, ' ');
				}
				ss << string(square_space_col, ' ');
			}
			ss << '\n';
		}
		ss << string(square_space_row, '\n');
	}

	return ss.str();
}

int total_positions = 0;

void GamePosition::print_info_about_position(bool show_next_moves) const {
	auto props = this->props();

	int wmc = props.winning_move_count, lmc = props.losing_move_count;

	std::printf("Position %i (out of %i) is %s.\n", props.id, total_positions, props.is_winning ? "winning" : "losing");
	std::printf("Distance to game end, assuming optimal play: %i\nNumber of winning cuts: %i\nNumber of losing cuts: %i\nTotal number of cuts: %i\n", props.distance_to_end, wmc, lmc, wmc+lmc);
	if (show_next_moves && props.is_winning) {
		auto winning_moves = get_winning_moves();

		std::printf("\nWinning moves:\n");

		for (const auto &move : winning_moves) {
			Cut c = move.second;

			std::printf("Cutting at (%i, %i) leads to a win in at most %i moves.", c.first + 1, c.second + 1, move.first.props().distance_to_end);
		}

		std::printf("\n");
	}
}

// Our strategy
// If it's your turn on a position p, it's either winning or losing. For example,
// [0, 0, 1] is losing, while [0, 0, 2] is winning. A position is winning if
// one of its cuts goes to a losing position, and is losing if all of its cuts go to
// a winning position. We treat the empty board as a base case, a winning position.
// Indeed, all of the valid cuts of [0, 0, 1], which is a losing position, go to the
// empty board, a winning position.
// How do we keep track of whether a position is winning? We use an associative map,
// GamePosition -> PositionProperties. The harder question is determining whether
// it's winning. We note that in a game, the number of tiles strictly decreases.
// Thus, if we know the winning/losing status of all positions with less tiles than
// the position under consideration, we can determine its win/lose status rapidly.
// We therefore need to iterate through positions in order of how many tiles they
// have. A naive BFS adding one tile each time would result in *tons* of duplicate
// evaluations, so instead we generate the positions directly from the number of tiles.

template <typename L>
void get_positions_with_n_tiles(int t, GamePosition& p, L ret, int x=height, int h=width) {
	// We iterate through the values. If there are x rows left and t tiles to fill them, with a maximum value of h, then on the current row, we need to use at least d_min tiles and at most d_max tiles.
	// t - d_min <= (x - 1) * d_min; d_min >= 0: t <= x * d_min, d_min = max(t // x, 0)
	// t - d_max >= 0; d_max <= h: d_max = min(t, h)

	int d_min = std::max(t / x, 0); // floor division
	int d_max = std::min(t, h);

	for (int d = d_min; d <= d_max; ++d) {
		p.rows[height - x] = d;

		if (x == 1 || d == t) // if d == t then everything after is just 0
			ret(p);
		else
			get_positions_with_n_tiles(t - d, p, ret, x - 1, d);
	}

	p.rows[height - x] = 0;
}

template <typename Lambda>
void for_all_positions(Lambda lambda, Condition condition) {
	GamePosition store;

	for (int n = 1; n < width * height; ++n) {
		get_positions_with_n_tiles(n, store, [&] (GamePosition p) {
			switch (condition) {
				case all:
					lambda(p);
					return;
				case winning:
				case losing:
					if (p.is_winning() ^ (condition != winning)) {
						lambda(p);
					}
			}
		});
	}
}


int total_winning_positions = 0;

void construct_position_data() {
	using namespace std::chrono;

	GamePosition starting_position;

	int number_of_tiles;
	const int max_number_of_tiles = width * height;

	GamePosition position{};
	position.rows.fill(0);

	position_map[position] = PositionProperties(true, 0);

	auto time = high_resolution_clock::now();

	for (int n = 1; n <= max_number_of_tiles; ++n) {
		if (VERBOSE > 0) std::printf("Searching positions with %d tiles.\n", n);

		get_positions_with_n_tiles(n, position, [&] (GamePosition under_consideration) {
			if (VERBOSE > 1) {
				std::printf("Position %d:\n", total_positions);
				std::printf("%s\n", under_consideration.to_string().c_str());
			}

			int min_loss_d_to_end = width * height + 1;
			int max_d_to_end = -1;
			int losing_move_count = 0; // how many moves lose
			int winning_move_count = 0; // how many moves win
			bool is_winning = false;

			auto moves = under_consideration.get_moves();

			std::for_each(moves.begin(), moves.end(), [&] (auto move) {
				auto properties = move.first.props();

				int d_to_end = properties.distance_to_end;

				if (!properties.is_winning) {
					is_winning = true;
					winning_move_count++;
					min_loss_d_to_end = std::min(min_loss_d_to_end, d_to_end);
				} else {
					losing_move_count++;
				}

				max_d_to_end = std::max(max_d_to_end, d_to_end);
			});

			if (VERBOSE > 1) std::printf("is_winning: %s\n", is_winning ? "yes" : "no");

			auto set = position_map[under_consideration] = {
							is_winning,
							// if winning we want the minimum distance among subsequent LOSING moves. If losing we want the maximum distance among all moves (which are all winning).
							(is_winning ? min_loss_d_to_end : max_d_to_end) + 1,
							winning_move_count,
							losing_move_count,
							total_positions + 1
			};

			if (is_winning) total_winning_positions++;

			total_positions++;
		});
	}

	auto time_end = high_resolution_clock::now();

	if (VERBOSE > 0) std::printf("Processed %i positions in %d ms.\n", total_positions, (int)duration_cast<milliseconds>(time_end - time).count());
}

int main() {
	construct_position_data();

	/*for_all_positions([&] (GamePosition p) {
		if (p.get_losing_moves().size() > 6) {
			p.print_info_about_position();
		}
	}, winning);*/

	std::printf("Total winning: %i, total losing: %i.\n", total_winning_positions, total_positions - total_winning_positions);

	/*for (int rows = 1; rows <= height; ++rows) {
		for (int cols = rows; cols <= width; ++cols) {
			if (rows == cols && rows == 1) continue;

			std::array<int, height> row_values;

			for (int i = 0; i < height; ++i) {
				row_values[i] = (i < rows) ? cols : 0;
			}

			GamePosition pos {row_values};

			auto moves = pos.get_winning_moves();

			if (moves.size() > 0) {
				auto move = moves[0];

				std::printf("Winning move for board of size (rows by cols) = (%i by %i) is cutting at (row, col) = (%i, %i).\n", rows, cols, move.second.first + 1, move.second.second + 1);
			} else {
				std::printf("Board of size %i by %i is losing.\n", rows, cols);
			}
		}
	}*/

	GamePosition position = GamePosition::starting_position();
	std::vector<GamePosition> position_history;

	auto set_position = [&] (GamePosition p) {
		position = p;
		position_history.push_back(p);
	};

	auto undo = [&] () {
		if (position_history.empty()) {
			std::printf("Nothing to undo.\n");
		} else {
			position = position_history.back();
			position_history.pop_back();
		}
	};

	auto cut_current_position = [&] (Cut c) {
		int row = c.first, col = c.second;
		std::printf("Cutting at (%i, %i).\n", row + 1, col + 1);

		if (position.is_valid_cut({row, col})) {
			set_position(position.cut({row, col}));
		} else {
			std::printf("Invalid cut.\n");
		}
	};

	return 0;

	while (true) {
		// q: quit program
		// w: automatically play winning move, or losing move that delays loss the longest
		// r: reset
		// c <row> <col>: play a cut
		// s <row 1 count> <row 2 count> ... : set the number of tiles in each row
		// i: print all info about a position
		// iw: print all info about a position, including all the winning moves.
		// u: undo; go to the previous position

		std::printf("%s", position.pretty_print().c_str());
		std::printf("> ");

		std::string inp;
		getline(std::cin, inp);

		if (inp.length() == 0) continue;

		switch (inp[0]) {
			case 'q':
				return 0;
			case 'r':
				set_position(GamePosition::starting_position());
				break;
			case 's': {
				std::stringstream ss(inp.substr(1));
				std::vector<int> v;

				int n;
				while (ss >> n) v.push_back(n);

				if (v.size() < height) {
					std::printf("Not enough numbers inputted; need %i numbers.\n", height);
					break;
				}

				std::array<int, height> rows;
				std::copy_n(v.begin(), height, rows.begin());

				if (is_valid_position(rows))
					set_position(GamePosition(rows));
				else
					std::printf("Invalid numbers inputted.\n");
				break;
			}
			case 'u':
				undo();
				break;
			case 'i':
				position.print_info_about_position();
				break;
			case 'c': {
				std::stringstream ss(inp.substr(1));
				int row = -1, col = -1;
				ss >> row >> col;

				if (row == -1 || col == -1) {
					std::printf("Unknown cut input.\n");
					break;
				}
				cut_current_position({row - 1, col - 1});

				break;
			}
			case 'w': {
				auto winning_moves = position.get_winning_moves();

				if (winning_moves.empty()) {
					std::printf("No winning move.\n");
					/*auto cut = position.get_losing_cut_longest_delay();

					cut_current_position(cut.value());*/
				} else {
					cut_current_position(winning_moves[0].second);
				}
				break;
			}
			default:
				std::printf("Unknown input.\n");
		}
	}
}