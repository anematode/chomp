/*
* @Author: UnsignedByte
* @Date:   2021-11-01 18:59:56
* @Last Modified by:   UnsignedByte
* @Last Modified time: 2021-11-02 13:22:33
*/

#include <position.h>

namespace Chomp {
	namespace store {
		void write_map(const std::unordered_map<uint64_t, Chomp::PositionInfo> &map, const char *filename) {
			FILE *f = fopen(filename, "w");
			if (!f) throw new std::runtime_error("Failed to open file");

			// std::setvbuf(f, NULL, _IOFBF, sizeof(buf));

			for (const auto &element : map) {
				fwrite(&element.first, sizeof(element.first), 1, f);
				element.second.store(f);
				// uint16_t position_info = element.second.dte | (0/*element.second.is_winning*/) << 15;
				// fwrite(&position_info, sizeof(position_info), 1, f);
			}

			fclose(f);
		}

		void read_map(std::unordered_map<uint64_t, Chomp::PositionInfo> &map, const char *filename) {
			FILE *f = fopen(filename, "r");
			if (!f) throw new std::runtime_error("Failed to open file");

			while (1) {
				uint64_t key;
				if (fread(&key, sizeof(key), 1, f) == 0) break;

				map[key] = Chomp::load_position_info(f);
			}

			fclose(f);
		}
	}
}