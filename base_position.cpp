#include "base_position.hpp"

#include <iomanip>
#include <vector>
#include <sstream>
#include <unordered_map>

#define FILE_LINE CHOMP_FILE_LINE
#define DEBUG CHOMP_DEBUG_VARS
#define DEBUG_NB CHOMP_DEBUG_VARS_NO_BRACES

namespace Chomp {
	/**
	 * HASHING
	 */
	const uint64_t HASH_PRIME = 179424673;

	hash_type hash_position(const int* rows, int height) {
		hash_type hash = 0;

		for (int i = 0; i < height; ++i) {
			hash += rows[i];
			hash *= HASH_PRIME;
		}

		return hash;
	}

	// Return the hash of the flipped position
	hash_type hash_position_flipped(const int* rows, int height) {
		int col = 0;
		hash_type hash = 0;

		for (int i = height - 1; i >= 0; --i) {
			int row = rows[i];

			while (row > col) {
				hash += i + 1;
				hash *= HASH_PRIME;
				col++;
			}
		}

		return hash;
	}

	/**
	 * COMBINATORICS
	 */

	// Precomputed for convenience
	static std::array<p_count_type, 417> partitions = {
		1ULL, 1ULL, 2ULL, 3ULL, 5ULL, 7ULL, 11ULL, 15ULL, 22ULL, 30ULL, 42ULL, 56ULL, 77ULL, 101ULL, 135ULL, 176ULL, 231ULL,
		297ULL, 385ULL, 490ULL, 627ULL, 792ULL, 1002ULL, 1255ULL, 1575ULL, 1958ULL, 2436ULL, 3010ULL, 3718ULL, 4565ULL,
		5604ULL, 6842ULL, 8349ULL, 10143ULL, 12310ULL, 14883ULL, 17977ULL, 21637ULL, 26015ULL, 31185ULL, 37338ULL, 44583ULL,
		53174ULL, 63261ULL, 75175ULL, 89134ULL, 105558ULL, 124754ULL, 147273ULL, 173525ULL, 204226ULL, 239943ULL, 281589ULL,
		329931ULL, 386155ULL, 451276ULL, 526823ULL, 614154ULL, 715220ULL, 831820ULL, 966467ULL, 1121505ULL, 1300156ULL,
		1505499ULL, 1741630ULL, 2012558ULL, 2323520ULL, 2679689ULL, 3087735ULL, 3554345ULL, 4087968ULL, 4697205ULL,
		5392783ULL, 6185689ULL, 7089500ULL, 8118264ULL, 9289091ULL, 10619863ULL, 12132164ULL, 13848650ULL, 15796476ULL,
		18004327ULL, 20506255ULL, 23338469ULL, 26543660ULL, 30167357ULL, 34262962ULL, 38887673ULL, 44108109ULL, 49995925ULL,
		56634173ULL, 64112359ULL, 72533807ULL, 82010177ULL, 92669720ULL, 104651419ULL, 118114304ULL, 133230930ULL,
		150198136ULL, 169229875ULL, 190569292ULL, 214481126ULL, 241265379ULL, 271248950ULL, 304801365ULL, 342325709ULL,
		384276336ULL, 431149389ULL, 483502844ULL, 541946240ULL, 607163746ULL, 679903203ULL, 761002156ULL, 851376628ULL,
		952050665ULL, 1064144451ULL, 1188908248ULL, 1327710076ULL, 1482074143ULL, 1653668665ULL, 1844349560ULL,
		2056148051ULL, 2291320912ULL, 2552338241ULL, 2841940500ULL, 3163127352ULL, 3519222692ULL, 3913864295ULL,
		4351078600ULL, 4835271870ULL, 5371315400ULL, 5964539504ULL, 6620830889ULL, 7346629512ULL, 8149040695ULL,
		9035836076ULL, 10015581680ULL, 11097645016ULL, 12292341831ULL, 13610949895ULL, 15065878135ULL, 16670689208ULL,
		18440293320ULL, 20390982757ULL, 22540654445ULL, 24908858009ULL, 27517052599ULL, 30388671978ULL, 33549419497ULL,
		37027355200ULL, 40853235313ULL, 45060624582ULL, 49686288421ULL, 54770336324ULL, 60356673280ULL, 66493182097ULL,
		73232243759ULL, 80630964769ULL, 88751778802ULL, 97662728555ULL, 107438159466ULL, 118159068427ULL, 129913904637ULL,
		142798995930ULL, 156919475295ULL, 172389800255ULL, 189334822579ULL, 207890420102ULL, 228204732751ULL,
		250438925115ULL, 274768617130ULL, 301384802048ULL, 330495499613ULL, 362326859895ULL, 397125074750ULL,
		435157697830ULL, 476715857290ULL, 522115831195ULL, 571701605655ULL, 625846753120ULL, 684957390936ULL,
		749474411781ULL, 819876908323ULL, 896684817527ULL, 980462880430ULL, 1071823774337ULL, 1171432692373ULL,
		1280011042268ULL, 1398341745571ULL, 1527273599625ULL, 1667727404093ULL, 1820701100652ULL, 1987276856363ULL,
		2168627105469ULL, 2366022741845ULL, 2580840212973ULL, 2814570987591ULL, 3068829878530ULL, 3345365983698ULL,
		3646072432125ULL, 3972999029388ULL, 4328363658647ULL, 4714566886083ULL, 5134205287973ULL, 5590088317495ULL,
		6085253859260ULL, 6622987708040ULL, 7206841706490ULL, 7840656226137ULL, 8528581302375ULL, 9275102575355ULL,
		10085065885767ULL, 10963707205259ULL, 11916681236278ULL, 12950095925895ULL, 14070545699287ULL, 15285151248481ULL,
		16601598107914ULL, 18028182516671ULL, 19573856161145ULL, 21248279009367ULL, 23061871173849ULL, 25025873760111ULL,
		27152408925615ULL, 29454549941750ULL, 31946390696157ULL, 34643126322519ULL, 37561133582570ULL, 40718063627362ULL,
		44132934884255ULL, 47826239745920ULL, 51820051838712ULL, 56138148670947ULL, 60806135438329ULL, 65851585970275ULL,
		71304185514919ULL, 77195892663512ULL, 83561103925871ULL, 90436839668817ULL, 97862933703585ULL, 105882246722733ULL,
		114540884553038ULL, 123888443077259ULL, 133978259344888ULL, 144867692496445ULL, 156618412527946ULL,
		169296722391554ULL, 182973889854026ULL, 197726516681672ULL, 213636919820625ULL, 230793554364681ULL,
		249291451168559ULL, 269232701252579ULL, 290726957916112ULL, 313891991306665ULL, 338854264248680ULL,
		365749566870782ULL, 394723676655357ULL, 425933084409356ULL, 459545750448675ULL, 495741934760846ULL,
		534715062908609ULL, 576672674947168ULL, 621837416509615ULL, 670448123060170ULL, 722760953690372ULL,
		779050629562167ULL, 839611730366814ULL, 904760108316360ULL, 974834369944625ULL, 1050197489931117ULL,
		1131238503938606ULL, 1218374349844333ULL, 1312051800816215ULL, 1412749565173450ULL, 1520980492851175ULL,
		1637293969337171ULL, 1762278433057269ULL, 1896564103591584ULL, 2040825852575075ULL, 2195786311682516ULL,
		2362219145337711ULL, 2540952590045698ULL, 2732873183547535ULL, 2938929793929555ULL, 3160137867148997ULL,
		3397584011986773ULL, 3652430836071053ULL, 3925922161489422ULL, 4219388528587095ULL, 4534253126900886ULL,
		4872038056472084ULL, 5234371069753672ULL, 5622992691950605ULL, 6039763882095515ULL, 6486674127079088ULL,
		6965850144195831ULL, 7479565078510584ULL, 8030248384943040ULL, 8620496275465025ULL, 9253082936723602ULL,
		9930972392403501ULL, 10657331232548839ULL, 11435542077822104ULL, 12269218019229465ULL, 13162217895057704ULL,
		14118662665280005ULL, 15142952738857194ULL, 16239786535829663ULL, 17414180133147295ULL, 18671488299600364ULL,
		20017426762576945ULL, 21458096037352891ULL, 23000006655487337ULL, 24650106150830490ULL, 26415807633566326ULL,
		28305020340996003ULL, 30326181989842964ULL, 32488293351466654ULL, 34800954869440830ULL, 37274405776748077ULL,
		39919565526999991ULL, 42748078035954696ULL, 45772358543578028ULL, 49005643635237875ULL, 52462044228828641ULL,
		56156602112874289ULL, 60105349839666544ULL, 64325374609114550ULL, 68834885946073850ULL, 73653287861850339ULL,
		78801255302666615ULL, 84300815636225119ULL, 90175434980549623ULL, 96450110192202760ULL, 103151466321735325ULL,
		110307860425292772ULL, 117949491546113972ULL, 126108517833796355ULL, 134819180623301520ULL, 144117936527873832ULL,
		154043597379576030ULL, 164637479165761044ULL, 175943559810422753ULL, 188008647052292980ULL, 200882556287683159ULL,
		214618299743286299ULL, 229272286871217150ULL, 244904537455382406ULL, 261578907351144125ULL, 279363328483702152ULL,
		298330063062758076ULL, 318555973788329084ULL, 340122810048577428ULL, 363117512048110005ULL, 387632532919029223ULL,
		413766180933342362ULL, 441622981929358437ULL, 471314064268398780ULL, 502957566506000020ULL, 536679070310691121ULL,
		572612058898037559ULL, 610898403751884101ULL, 651688879997206959ULL, 695143713458946040ULL, 741433159884081684ULL,
		790738119649411319ULL, 843250788562528427ULL, 899175348396088349ULL, 958728697912338045ULL, 1022141228367345362ULL,
		1089657644424399782ULL, 1161537834849962850ULL, 1238057794119125085ULL, 1319510599727473500ULL,
		1406207446561484054ULL, 1498478743590581081ULL, 1596675274490756791ULL, 1701169427975813525ULL,
		1812356499739472950ULL, 1930656072350465812ULL, 2056513475336633805ULL, 2190401332423765131ULL,
		2332821198543892336ULL, 2484305294265418180ULL, 2645418340688763701ULL, 2816759503217942792ULL,
		2998964447736452194ULL, 3192707518433532826ULL, 3398704041358160275ULL, 3617712763867604423ULL,
		3850538434667429186ULL, 4098034535626594791ULL, 4361106170762284114ULL, 4640713124699623515ULL,
		4937873096788191655ULL, 5253665124416975163ULL, 5589233202595404488ULL, 5945790114707874597ULL,
		6324621482504294325ULL, 6727090051741041926ULL, 7154640222653942321ULL, 7608802843339879269ULL,
		8091200276484465581ULL, 8603551759348655060ULL, 9147679068859117602ULL, 9725512513742021729ULL,
		10339097267123947241ULL, 10990600063775926994ULL, 11682316277192317780ULL, 12416677403151190382ULL,
		13196258966925435702ULL, 14023788883518847344ULL, 14902156290309948968ULL, 15834420884488187770ULL,
		16823822787139235544ULL, 17873792969689876004ULL // next is 18987964267331664557, beyond 2^64
	};

  // Let p(n) be the partition function. Let p(N, M; n) be the number of positions with n tiles in an MxN rectangle.
	// Clearly p(n) >= p(N, M; n) and if N,M <= n, p(n) = p(N, M; n). p(N,M;n) = p(N,M-1;n)+p(N-1,M,n-M), and
	// p(N,M;n)=p(M,N;n). The total number of positions in an MxN rectangle is (M + N choose N); to see this, note that
	// the boundary of a given position travels up and to the left in a series of stages, say M movements up and N
	// movements left, which can be rearranged in (M + N choose N) distinct ways -- each of which corresponds with a
	// unique position (including the empty position)

	// self explanatory
	p_count_type partition_function(int n) {
		return partitions.at(n);
	}

	// Sum p(n) from min to max, INCLUSIVE
	p_count_type partition_function_sum(int min, int max) {
		p_count_type result = 0;

		for (int i = min; i <= max; ++i) {
			result += partition_function(i);
			if (result < 0) {
				// Underflow
				throw std::runtime_error(FILE_LINE + "Overflow calling partition_function_sum with parameters " + DEBUG(min, max));
			}
		}

		return result;
	}

	// Overflow not handled, but is unlikely for small arguments
	p_count_type choose_function(int n, int r) {
		// nCr(n, r) = n/1 * (n-1)/2 * (n-2)/3 * ...

		if (r == 0) return 1;
		if (r > n / 2) return choose_function(n, n-r);

		p_count_type result = 1;
		for (int k = 1; k <= r; ++k) {
			result *= n - k + 1;
			result /= k;
		}

		return result;
	}

	p_count_type unevaluated_value = -1; // dummy value; will underflow to 2^64 - 1

	// Cache values of rectangle_partition_count for (n, width, height) where width <= height (since order does not matter)
	// We store the lesser dimension first, then the greater dimension, then the number n. Each is a vector. A value of
	// unevaluated_value signifies the value has not yet been computed.
	std::vector<std::vector<std::vector<p_count_type>>> cached_partition_counts = {};

	// Get the number of partitions of n that fit in the given rectangle, caching/memoizing the results
	p_count_type rectangle_partition_count(int n, int width, int height) {
		if (width < 0 || height < 0) return 0;
		if (n == 0) return 1;
		if (n < 0) return 0;

		if (width > height) std::swap(width, height); // ;)
		// Various base cases
		if (width == 0) return 0; // since n != 0
		if (width == 1) return n <= height; // 1 if true
		if (width == 2) {
			int area = height * 2;
			if (n > area) return 0;
			if (n <= height) return (n + 2) / 2;
			return ((area - n) + 2) / 2;
		}

		if (n <= width) return partition_function(n);

		int area = width * height;
		if (n == area) return 1;
		if (n > area) return 0;

		if (cached_partition_counts.size() <= width) {
			cached_partition_counts.resize(width + 1);
			cached_partition_counts[width] = std::vector<std::vector<p_count_type>>{};
		}

		auto& heights = cached_partition_counts[width];
		if (heights.size() <= height) {
			heights.resize(height + 1);
			heights[height] = std::vector<p_count_type>{};
		}

		auto& values = heights[height];
		if (values.size() <= n) {
			size_t size = values.size();
			values.resize(n + 1);

			auto end = values.begin() + size;

			std::fill(end, values.end(), unevaluated_value);
		}

		// p(n, w, h) = p(n, w-1, h) + p(n-w, w, h-1) so we want to reduce this quickly to cases where
		// n <= width and n <= height.
		if (values[n] == unevaluated_value)
			values[n] = rectangle_partition_count(n, width, height-1) + rectangle_partition_count(n-height, width-1, height);

		return values[n];
	}

	p_count_type count_positions_inner(int min_squares, int max_squares, int bound_width, int bound_height) {
		// Assumptions: min_squares >= 0, min_squares <= max_squares, bound_width >= 0, bound_height >= 0
		// We can handle some special cases...

		if (bound_width == 0 || bound_height == 0) {
			return min_squares == 0; // 1 if we include the empty position
		} else if (bound_width == 1) {
			return std::max(std::min(max_squares, bound_height) - min_squares + 1, 0);
		} else if (bound_height == 1) {
			return std::max(std::min(max_squares, bound_width) - min_squares + 1, 0);
		} else if (max_squares <= bound_width && max_squares <= bound_height) {
			return partition_function_sum(min_squares, max_squares);
		}

		// In the case of a board where min_squares fits as a partition and max_squares fits as an "inverse partition", we
		// can use the fact that an mxn board has (m + n choose m) possibilities and then subtract the corresponding
		// partitions. Illustration:
		//    #####XX
		//    ######X
		//    #######
		// 18 squares, 7x3 grid; Xs form an "inverse partition".

		int area = bound_width * bound_height;
		if (min_squares <= bound_width && min_squares <= bound_height && max_squares >= (area - std::min(bound_width, bound_height))) {
			return choose_function(bound_width + bound_height, bound_height) // all positions
			 - partition_function_sum(0, min_squares - 1) // exclude bottom-left partitions
			 - partition_function_sum(0, area - max_squares - 1); // exclude top-right "inverse partitions"
		}

		if (min_squares == max_squares) return rectangle_partition_count(min_squares, bound_width, bound_height);

		// Break down the problem recursively
		int middle = (min_squares + max_squares) / 2;
		return count_positions_inner(min_squares, middle /* since floor division */, bound_width, bound_height)
			+ count_positions_inner(middle + 1, max_squares, bound_width, bound_height);
	}

	// Function is pretty fast
	p_count_type count_positions(int min_squares, int max_squares, int bound_width, int bound_height) {
		if (max_squares == -1 && (bound_width == -1 || bound_height == -1))
			throw std::runtime_error(FILE_LINE + "Cannot have unbounded square count and dimensions; " + DEBUG(max_squares, bound_width, bound_height));

		if (bound_width == -1) bound_width = INT_MAX;
		if (bound_height == -1) bound_height = INT_MAX;

		min_squares = std::max(0, min_squares);

		if (max_squares < min_squares || max_squares < 0) return 0;
		if (max_squares == 0) return 1;

		return count_positions_inner(min_squares, max_squares, bound_width, bound_height);
	}

	/**
	 * FORMATTING AND RELATED CODE
	 */

	std::unordered_map<std::string, PositionFormatOptions> styles {
		{"default", {}},
		{"austere", { .tile_size=1, .sep=0, .show_labels=false }}
	};

	void PositionFormatOptions::set_default(PositionFormatOptions opts) {
		default_format_options = opts;
	}

	void PositionFormatOptions::set_default(std::string style) {
		if (!styles.contains(style)) throw std::runtime_error(FILE_LINE + "Unrecognized format style " + style);

		default_format_options = styles[style];
	}

	int PositionFormatOptions::get_vertical_sep() {
		return (sep < 0) ? vertical_sep : sep;
	}

	int PositionFormatOptions::get_horizontal_sep() {
		return (sep < 0) ? horizontal_sep : sep;
	}

	int PositionFormatOptions::get_tile_width() {
		return (tile_size < 0) ? tile_width : tile_size;
	}

	int PositionFormatOptions::get_tile_height() {
		return (tile_size < 0) ? tile_height : tile_size;
	}

	// TODO: make more robust
	std::string _position_to_string (int* rows, int height, PositionFormatOptions opts) {
		using namespace std;
		int width = rows[0];
		if (height < 0 || height >= 1000 || width < 0) return "<invalid position>";

		// The basic unit of printing is a rectangle of size tile_width x tile_height, which we store as a sequence of chars
		// unbroken by newlines

		int tile_width = opts.get_tile_width();
		int tile_height = opts.get_tile_height();
		int tile_area = tile_width * tile_height;

		int horizontal_sep = opts.get_horizontal_sep();
		int vertical_sep = opts.get_vertical_sep();

		string empty_tile = string(tile_area, opts.empty_char);
		string filled_tile = string(tile_area, opts.tile_char);

		int print_width = max(opts.min_width, width);
		int print_height = max(opts.min_height, height);

		// List of rows, top to bottom
		vector<vector<string>> out;

		// Get an appropriate string for a left marker on a given row
		auto row_marker = [&] (int r) {
			string marker = string(tile_area, ' ');
			string as_str = std::to_string(r);

			// Center it vertically and right-align it
			marker.insert((tile_area + 1) / 2 + tile_width - as_str.length() - 1, as_str);
			return marker;
		};

		// Get an appropriate string for a top marker on a given column
		auto col_marker = [&] (int c) {
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
				out[row].insert(out[row].begin(), row_marker(print_height - row - 1)); // invert the labels
			}

			vector<string> first_row; // new first row
			first_row.push_back(string(tile_area, ' ')); // spacer

			for (int col = 0; col < print_width; ++col)
				first_row.push_back(col_marker(col));

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

#undef FILE_LINE
#undef DEBUG
#undef DEBUG_NB