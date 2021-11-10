#pragma once

#include "base_position.hpp"
#include "base_atlas.hpp"
#include "parallel_hashmap/phmap.h"

namespace Chomp {
	size_t dims_to_index(int width, int height) {
		// Assumes width >= height; mapping between unordered pairs and natural numbers
		int s = width - height - 1;

		return width * height + (s * s / 4); // floor division
	}

	struct ProcessResult {
		int losing_positions = -1;
	};

	// Stores a set of positions. Generally an atlas will have all positions in a rectangle or all positions with a given
	// square count or less. Note that the atlas may not necessarily store all the positions; for space efficiency, the
	// losing positions may sometimes only be stored
	template<int MAX_HEIGHT>
	class Atlas : public BaseAtlas<MAX_HEIGHT> {
	public:
		using Base = BaseAtlas<MAX_HEIGHT>;
		using typename Base::Position; // allow us to use Position
		using hash_map = phmap::parallel_flat_hash_map<hash_type, PositionInfo>;
		using Base::is_position_known;
		using Base::get_position_info;
		using Base::store_position_info;

		PositionInfo get_position_info(Position& p) {
			if (p.height() == 0) return { .is_winning = WINNING::YES, .distance_to_end = 0 };
			/*if (!is_position_known(p))
				return {};*/
			auto map = get_hash_map(p);
			auto iterator = map->find(p.canonical_hash());

			if (iterator == map->end()) {
				return { .is_winning = WINNING::YES };
			} else {
				return iterator->second;
			}
		}

		std::pair<size_t, size_t> get_position_hash_location(Position& p) {
			int width = p.width(), height = p.height();
			if (!p.is_canonical()) std::swap(width, height);

			size_t index = dims_to_index(width, height);
			return { index, p.square_count() - width - height + 1 };
		}

		hash_map* get_hash_map(Position& p) {
			auto [ dim_index, sub_index ] = get_position_hash_location(p);

			data.resize(std::max(dim_index + 1, data.size()));

			auto v = data[dim_index];
			if (v == nullptr) v = data[dim_index] = new std::vector<hash_map*>;

			v->resize(std::max(sub_index + 1, v->size()));
			for (int i = v->size(); i < sub_index + 1; ++i) {
				v->at(i) = nullptr;
			}

			auto map = v->at(sub_index);
			if (map == nullptr) map = v->at(sub_index) = new hash_map{};

			return map;
		}

		// A position is found at the vector index in the data, then at the position corresponding to its square count
		virtual void store_position_info(Position &p, PositionInfo info) {
			auto& map = *get_hash_map(p);

			map[p.canonical_hash()] = info;
		}

		void hash_positions(int min_squares, int max_squares, int bound_width=-1, int bound_height=-1) {
			std::vector<Position> positions;
			const int POSITION_BATCH_SIZE = 1000000;

			int losing_positions = 0;

			auto process_positions = [&] {
				ProcessResult res = hash_positions_over_iterator(positions.begin(), positions.end());
				losing_positions += res.losing_positions;

				positions.clear();
			};

			min_squares = std::max(min_squares, 1);

			for (int n = min_squares; n <= max_squares; ++n) {
				Position::positions_with_n_tiles(n, [&] (Position &p) {
					positions.push_back(p);

					// Process positions in batches
					if (positions.size() > POSITION_BATCH_SIZE) {
						process_positions();
					}
				}, bound_width, bound_height, true /* only canonical positions */);

				process_positions();
				std::cout << n << ' ' << losing_positions << '\n';
				losing_positions = 0;
			}
		}

		~Atlas() {
			for (auto p : data) {
				if (p != nullptr) {
					for (auto map : *p)
						if (map != nullptr)
							delete map;

					delete p;
				}
			}
		}
	private:
		std::vector<std::vector<hash_map*>*> data;

		using position_iterator = typename std::vector<Position>::iterator;
		ProcessResult hash_positions_over_iterator(position_iterator begin, position_iterator end) {
			int losing_positions = 0;

			std::vector<Position> cutted_list;

			for (auto it = begin; it != end; ++it) {
				Position p = *it;

				bool is_winning = false;
				int multiplicity = p.multiplicity();

				p.get_cuts([&] (Cut c) {
					Position cutted = p.cut(c);

					if (get_position_info(cutted).is_winning == WINNING::NO) {
						is_winning = true;
						return true;
					}

					return false;
				});

				int max_dte = 0;

				if (!is_winning) {
					store_position_info(p, { .is_winning = WINNING::NO, .distance_to_end = max_dte });
					losing_positions += multiplicity;
				}

				cutted_list.clear();
			}

			return { .losing_positions = losing_positions };
		}
	};
}