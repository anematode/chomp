/*
* @Author: UnsignedByte
* @Date:   2021-11-01 18:59:56
* @Last Modified by:   UnsignedByte
* @Last Modified time: 2021-11-01 20:00:11
*/

#include <position.h>

namespace Chomp {
	namespace store {
		void write_map(const std::unordered_map<uint64_t, Chomp::LosingPositionInfo> &map, const char *filename) {
			FILE *f = fopen(filename, "w");
			if (!f) throw new std::runtime_error("Failed to open file");

			// std::setvbuf(f, NULL, _IOFBF, sizeof(buf));

			for (const auto &element : map) {
				fwrite(&element.first, sizeof(element.first), 1, f);
				uint16_t position_info = element.second.dte | (0/*element.second.is_winning*/) << 15;
				fwrite(&position_info, sizeof(position_info), 1, f);
			}

			fclose(f);
		}

		void read_map(std::unordered_map<uint64_t, Chomp::LosingPositionInfo> &map, const char *filename) {
			FILE *f = fopen(filename, "r");
			if (!f) throw new std::runtime_error("Failed to open file");

			while (1) {
				uint64_t key;
				uint16_t v;
				if (fread(&key, sizeof(key), 1, f) == 0) break;
				fread(&v, sizeof(v), 1, f);

				map[key] = Chomp::LosingPositionInfo({
								                                     //.is_winning = false,//(bool)(v >> 15),
								                                     .dte = v & ((1 << 15) - 1)
				                                     });
			}

			fclose(f);
		}
	}
}