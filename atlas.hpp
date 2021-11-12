#pragma once

#include "base_position.hpp"
#include "base_atlas.hpp"
#include "parallel_hashmap/phmap.h"
#include <memory>
#include <thread>

namespace Chomp {
	/**
	 * Maps a width and height, assuming width >= height, to an index
	 * grows in O(wh)
	 * @param width
	 * @param height
	 * @return
	 */
	size_t dims_to_index(int width, int height) {
		int s = width - height - 1;

		return width * height + (s * s / 4); // floor division
	}

	/**
	 * Summary of the result of some computation
	 */
	struct ProcessResult {
		int losing_positions = -1;
	};

	/**
	 * Hashing strategy used by the atlas
	 */
	enum class HashingStrategy {
		DIMS_SQUARE_COUNT,
		DIMS_ONLY
	};

	// Stores a set of positions. Generally an atlas will have all positions in a rectangle or all positions with a given
	// square count or less. Note that the atlas may not necessarily store all the positions; for space efficiency, the
	// losing positions may sometimes only be stored
	template<int MAX_HEIGHT, HashingStrategy HASHING_STRATEGY=HashingStrategy::DIMS_SQUARE_COUNT>
	class Atlas final : public BaseAtlas<MAX_HEIGHT> {
		using Base = BaseAtlas<MAX_HEIGHT>;
	public:
		using typename Base::Position; // allow us to use Position
		using hash_map = phmap::parallel_flat_hash_map<hash_type, PositionInfo>;

		using Base::is_position_known;
		using Base::get_position_info;
		using Base::store_position_info;

		// DIMS_SQUARE_COUNT -> pairs, DIMS_ONLY -> single index
		using hash_location_type = std::conditional_t<HASHING_STRATEGY == HashingStrategy::DIMS_SQUARE_COUNT,
			std::pair<size_t, size_t>, size_t>;

		PositionInfo get_position_info(Position& p) const {
			if (p.height() == 0) return { .is_winning = WINNING::YES, .distance_to_end = 0 };
			/*if (!is_position_known(p))
				return {};*/
			auto map = get_hash_map_if_exists(p);
			if (map == nullptr) return { .is_winning = WINNING::UNKNOWN };

			auto iterator = map->find(p.canonical_hash());

			if (iterator == map->end()) {
				return { .is_winning = WINNING::YES };
			} else {
				return iterator->second;
			}
		}

		/**
		 * Get the location a given position would be found. If hashing dimensions and square count, we return a pair of
		 * indices; if only hashing dimensions, we return a single index
		 */
		hash_location_type get_position_hash_location(Position& p) const {
			int width = p.width(), height = p.height();
			if (!p.is_canonical()) std::swap(width, height);

			size_t index = dims_to_index(width, height);

			if constexpr (HASHING_STRATEGY == HashingStrategy::DIMS_SQUARE_COUNT) {
				return {index, p.square_count() - width - height + 1};
			} else {
				return index;
			}
		}

		// returns nullptr if doesn't exist
		hash_map* get_hash_map_at_location_if_exists(hash_location_type loc) const {
			if constexpr (HASHING_STRATEGY == HashingStrategy::DIMS_SQUARE_COUNT) {
				auto [dim_index, sub_index] = loc;

				if (data.size() <= dim_index)
					return nullptr;

				auto v = data[dim_index];
				if (v == nullptr || v->size() <= sub_index) return nullptr;

				auto map = v->at(sub_index);
				if (map == nullptr) return nullptr;

				return map;
			} else {
				if (data.size() <= loc) return nullptr;

				auto v = data[loc];
				return v;
			}
		}

		hash_map* get_hash_map_at_location(hash_location_type loc) {
			hash_map* map = get_hash_map_at_location_if_exists(loc);

			if (map == nullptr) {
				// Create a map
				if constexpr (HASHING_STRATEGY == HashingStrategy::DIMS_SQUARE_COUNT) {
					auto [dim_index, sub_index] = loc;

					data.resize(std::max(dim_index + 1, data.size()));

					auto v = data[dim_index];
					if (v == nullptr) {
						v = data[dim_index] = new std::vector<hash_map *>{};
					}

					v->resize(std::max(sub_index + 1, v->size()));

					auto map = v->at(sub_index);
					if (map == nullptr) map = v->at(sub_index) = new hash_map{};

					return map;
				} else {
					data.resize(std::max(loc + 1, data.size()));
					auto v = data[loc];
					if (v == nullptr) v = data[loc] = new hash_map{};

					return v;
				}
			}

			return map;
		}

		hash_map* get_hash_map(Position& p) {
			return get_hash_map_at_location(get_position_hash_location(p));
		}

		hash_map* get_hash_map_if_exists(Position& p) const {
			return get_hash_map_at_location_if_exists(get_position_hash_location(p));
		}

		// A position is found at the vector index in the data, then at the position corresponding to its square count
		virtual void store_position_info(Position &p, PositionInfo info) {
			auto& map = *get_hash_map(p);

			map[p.canonical_hash()] = info;
		}

		void hash_positions (int min_squares, int max_squares, int bound_width=-1, int bound_height=-1, size_t num_threads=1) {
			if (num_threads > 64)
				throw new std::runtime_error(CHOMP_FILE_LINE + "no");

			std::vector<Position> positions;
			const int POSITION_BATCH_SIZE = 1000000;
			const int MULTITHREAD_THRESHOLD = 10000;

			int losing_positions = 0;

			// Atlases for each thread
			std::vector<std::unique_ptr<Atlas>> thread_atlases;
			for (int i = 0; i < num_threads; ++i) {
				auto atlas = std::make_unique<Atlas>();
				thread_atlases.push_back(std::move(atlas));
			}

			using iterator_type = typename std::vector<Position>::iterator;

			auto process_positions = [&] {
				// Vector of results from each thread
				std::vector<ProcessResult> results{num_threads};
				std::vector<std::thread> threads;

				int actual_threads = num_threads;

				if (num_threads == 1 || positions.size() < MULTITHREAD_THRESHOLD) {
					results[0] = hash_positions_over_iterator(positions.begin(), positions.end(), *this);
					actual_threads = 1; // there was only one thread
				} else {
					iterator_type begin, end = positions.begin();
					size_t position_count = positions.size();

					for (size_t i = 0; i < num_threads; ++i) {
						// divvy up the positions
						begin = end;
						end = (i == num_threads - 1) ? positions.end() : (positions.begin() + ((i + 1) * position_count / num_threads));

						// Hash positions on each thread
						threads.emplace_back(std::thread([&, i, begin, end] {
							results[i] = thread_atlases[i]->hash_positions_over_iterator(begin, end, *this);
						}));
					}

					// Wait for all threads to finish
					for (size_t i = 0; i < num_threads; ++i) {
						threads[i].join();
					}

					// Merge all the atlases
					for (auto& atlas : thread_atlases) {
						merge(*atlas);
					}

					// Clear the atlases
					for (size_t i = 0; i < num_threads; ++i) {
						thread_atlases[i]->clear();
					}
				}

				// Add results
				for (int i = 0; i < actual_threads; ++i) {
					losing_positions += results[i].losing_positions;
				}

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

			mark_positions_as_computed(min_squares, max_squares, bound_width, bound_height);
		}

		void merge (const Atlas& atlas) {
			for (size_t dim_index = 0; dim_index < atlas.data.size(); ++dim_index) {
				auto p = atlas.data[dim_index];
				if (p != nullptr) {
					// Something to merge
					if constexpr (HASHING_STRATEGY == HashingStrategy::DIMS_SQUARE_COUNT) {
						for (size_t i = 0; i < p->size(); ++i) {
							auto map = p->at(i);
							if (map == nullptr) continue;

							// Merge map
							get_hash_map_at_location({dim_index, i})->merge(*map);
						}
					} else {
						// Merge map
						get_hash_map_at_location(dim_index)->merge(*p);
					}
				}
			}
		}

		// Empty an atlas
		void clear () {
			for (auto p : data) {
				if (p == nullptr) continue;

				if constexpr (HASHING_STRATEGY == HashingStrategy::DIMS_SQUARE_COUNT) {
					for (auto map : *p)
						if (map != nullptr)
							delete map;
				}

				delete p;
			}

			data.clear();
		}

		~Atlas() {
			for (auto p : data) {
				if (p != nullptr) {
					if constexpr (HASHING_STRATEGY == HashingStrategy::DIMS_SQUARE_COUNT) {
						for (auto map : *p)
							if (map != nullptr)
								delete map;
					}

					delete p;
				}
			}
		}
	private:
		// If hashing by dims and square count, vector of vectors; if hashing by dims only, single vector of maps
		using storage_type = std::conditional_t<HASHING_STRATEGY == HashingStrategy::DIMS_SQUARE_COUNT,
						std::vector<std::vector<hash_map*>*>, std::vector<hash_map*> >;
		using Base::mark_positions_as_computed;

		storage_type data;

		using position_iterator = typename std::vector<Position>::iterator;
		ProcessResult hash_positions_over_iterator(position_iterator begin, position_iterator end, const Atlas& reference_atlas) {
			int losing_positions = 0;

			for (auto it = begin; it != end; ++it) {
				Position& p = *it;

				bool is_winning = false;
				int multiplicity = p.multiplicity();

				p.get_potentially_winning_cuts([&] (Cut c) {
					Position cutted = p.cut(c);

					WINNING is_cutted_winning = reference_atlas.get_position_info(cutted).is_winning;

					if (is_cutted_winning == WINNING::NO) {
						is_winning = true;
						return true;
					}

					return false;
				});

				int max_dte = 0;

				if (!is_winning) {
					//std::cout << p.list() << '\n';
					store_position_info(p, { .is_winning = WINNING::NO, .distance_to_end = max_dte });
					losing_positions += multiplicity;
				}
			}

			return { .losing_positions = losing_positions };
		}
	};
}