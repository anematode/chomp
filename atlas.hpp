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

	// Stores a set of positions. Generally an atlas will have all positions in a rectangle or all positions with a given
	// square count or less. Note that the atlas may not necessarily store all the positions; for space efficiency, the
	// losing positions may sometimes only be stored
	template<int MAX_HEIGHT>
	class Atlas : BaseAtlas<MAX_HEIGHT> {
	public:
		using Base = BaseAtlas<MAX_HEIGHT>;
		using typename Base::Position; // allow us to use Position
		using hash_map = phmap::parallel_flat_hash_map<hash_type, PositionInfo>;
		using Base::is_position_known;

		PositionInfo get_position_info(Position& p) {
			if (p.height() == 0) return { .is_winning = WINNING::YES, .distance_to_end = 0 };
			/*if (!is_position_known(p))
				return {};*/
			return get_hash_map(p)->at(p.canonical_hash());
		}

		std::pair<size_t, size_t> get_position_hash_location(Position& p) {
			int width = p.width(), height = p.height();
			if (!p.is_canonical()) std::swap(width, height);

			size_t index = dims_to_index(width, height);
			return { index, p.square_count() - width - height }; // since there must be at least
		}

		hash_map* get_hash_map(Position& p) {
			auto [ dim_index, sub_index ] = get_position_hash_location(p);

			data.reserve(dim_index + 1);
			for (int i = data.size(); i < dim_index + 1; ++i) {
				data[i] = nullptr;
			}

			auto v = data[dim_index];
			if (v == nullptr) v = data.at(dim_index) = new std::vector<hash_map*>;

			v->reserve(sub_index + 1);
			for (int i = v->size(); i < sub_index + 1; ++i) {
				v->at(i) = nullptr;
			}

			auto map = v->at(sub_index);
			if (map == nullptr) map = v->at(sub_index) = new hash_map{};

			return map;
		}

		// A position is found at the vector index in the data, then at the position corresponding to its square count
		void store_position_info(Position &p, PositionInfo info) {
			auto map = get_hash_map(p);

			map->at(p.canonical_hash()) = info;
		}

		~Atlas() {
			for (auto p : data) {
				if (p != nullptr) {
					for (auto map : *p) {
						if (map != nullptr) {
							delete map;
						}
					}

					delete p;
				}
			}
		}
	private:
		std::vector<std::vector<hash_map*>*> data;
	};
}