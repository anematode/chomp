/*
* @Author: UnsignedByte
* @Date:   2021-11-02 16:34:08
* @Last Modified by:   UnsignedByte
* @Last Modified time: 2021-11-02 18:45:22
*/

#include <datastructs.hpp>

namespace Chomp {
	namespace datastructs {

		void BloomFilter::insert(uint64_t hash) {
			for (int i = 0; i < num_hash_functions; ++i) {
				bits.set(XXH::XXH64(i, hash) % bits.size());
			}
		}

		bool BloomFilter::probably_contains(uint64_t hash) const {
			for (int i = 0; i < num_hash_functions; ++i) {
				if (!bits.test(XXH::XXH64(i, hash) % bits.size())) return false;
			}

			return true;
		}

		namespace XXH {
			uint64_t XXH64(uint64_t seed, uint64_t data) {

			  uint64_t acc  = seed + PRIME64_5 + 8;

			  acc ^= ((data * PRIME64_2) << 31) * PRIME64_1;
	      acc = (acc << 27) * PRIME64_1 + PRIME64_4;

	      acc ^= (acc >> 33);
	      acc *= PRIME64_2;
	      acc ^= (acc >> 29);
				acc *= PRIME64_3;
				acc ^= (acc >> 32);

				return acc;
			}
		}
	}
}