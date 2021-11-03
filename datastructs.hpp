#include <bitset>

namespace Chomp {
	namespace datastructs {
		const unsigned long long max_bloom_size = 8589934592; // 1 Gigabyte
		const unsigned int num_hash_functions = 18; // https://hur.st/bloomfilter/?n=339699273&p=&m=8589934592&k=18

		class BloomFilter {
		private:
			std::bitset<max_bloom_size> bits;
		public:
			void insert(uint64_t hash);

			bool probably_contains(uint64_t hash) const;
		};

		namespace XXH {
			static const uint64_t PRIME64_1 = 0x9E3779B185EBCA87ULL;  // 0b1001111000110111011110011011000110000101111010111100101010000111
			static const uint64_t PRIME64_2 = 0xC2B2AE3D27D4EB4FULL;  // 0b1100001010110010101011100011110100100111110101001110101101001111
			static const uint64_t PRIME64_3 = 0x165667B19E3779F9ULL;  // 0b0001011001010110011001111011000110011110001101110111100111111001
			static const uint64_t PRIME64_4 = 0x85EBCA77C2B2AE63ULL;  // 0b1000010111101011110010100111011111000010101100101010111001100011
			static const uint64_t PRIME64_5 = 0x27D4EB2F165667C5ULL;  // 0b0010011111010100111010110010111100010110010101100110011111000101

			// XXH64 implementation for 64 bit int inputs (https://github.com/Cyan4973/xxHash/blob/dev/doc/xxhash_spec.md#xxh64-algorithm-description)
			uint64_t XXH64(uint64_t seed, uint64_t data);
		}
	}
}