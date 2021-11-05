//
// Created by Timothy Herchen on 11/4/21.
//

#pragma once

#include <tuple>
#include <iostream>

// For compiler portability
#undef INT_MAX
#define INT_MAX 2147483647

namespace {
	// Credit to https://stackoverflow.com/a/31172120/13458117
	constexpr bool is_path_sep(char c) {
		return c == '/' || c == '\\';
	}

	consteval const char* strip_path(const char* path) { // guaranteed to remove full file name at compile time
		auto lastname = path;
		for (auto p = path; *p; ++p) {
			if (is_path_sep(*p) && *(p+1)) lastname = p + 1;
		}
		return lastname;
	}
}

// Get the file and line number
#define CHOMP_STR_HELPER(x) #x
#define CHOMP_STR(x) CHOMP_STR_HELPER(x)
// Not necessarily compile-time evaluated, unfortunately
#define CHOMP_FILE_LINE std::string(strip_path(__FILE__)) + ':' + CHOMP_STR(__LINE__) + ": "

namespace {
	template <class T>
	std::string to_string(T a) {
		return std::to_string(a);
	}

	template<>
	std::string to_string<bool>(bool b) {
		return b ? "true" : "false";
	}

	template<int Index=0, class... Types>
	static std::string debugger_to_string(std::tuple<Types...> tuple) {
		if constexpr (Index < sizeof...(Types) - 1) {
			return to_string(std::get<Index>(tuple)) + ", " + debugger_to_string<Index + 1, Types...>(tuple);
		} else
			return to_string(std::get<Index>(tuple));
	}
}

// let's fucking go
#define CHOMP_SHOW_VAR1(a) CHOMP_STR_HELPER(a)
#define CHOMP_SHOW_VAR2(a, ...) CHOMP_STR_HELPER(a) ", " CHOMP_SHOW_VAR1(__VA_ARGS__)
#define CHOMP_SHOW_VAR3(a, ...) CHOMP_STR_HELPER(a) ", " CHOMP_SHOW_VAR2(__VA_ARGS__)
#define CHOMP_SHOW_VAR4(a, ...) CHOMP_STR_HELPER(a) ", " CHOMP_SHOW_VAR3(__VA_ARGS__)
#define CHOMP_SHOW_VAR5(a, ...) CHOMP_STR_HELPER(a) ", " CHOMP_SHOW_VAR4(__VA_ARGS__)
#define CHOMP_SHOW_VAR6(a, ...) CHOMP_STR_HELPER(a) ", " CHOMP_SHOW_VAR5(__VA_ARGS__)

// concat two strings
#define CONCATENATE_HELPER(a, b) a ## b
#define CONCATENATE(a, b) CONCATENATE_HELPER(a,b)

// Credit to https://stackoverflow.com/a/2124385/13458117
#define CHOMP_PP_NARG(...) \
         CHOMP_PP_NARG_(__VA_ARGS__,CHOMP_PP_RSEQ_N())
#define CHOMP_PP_NARG_(...) \
         CHOMP_PP_ARG_N(__VA_ARGS__)
#define CHOMP_PP_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define CHOMP_PP_RSEQ_N() 8,7,6,5,4,3,2,1,0

#define CHOMP_SHOW_VARS(...) CONCATENATE(CHOMP_SHOW_VAR,CHOMP_PP_NARG(__VA_ARGS__))(__VA_ARGS__)

// Diagnostics as string (for example, DEBUG_VARS(MAX_HEIGHT, n) might give "(MAX_HEIGHT, n) = (100, 5)"
#define CHOMP_DEBUG_VARS(...) std::string("(" CHOMP_SHOW_VARS(__VA_ARGS__) ") = (") + debugger_to_string(std::make_tuple(__VA_ARGS__)) + ')'
#define CHOMP_DEBUG_VARS_NO_BRACES(...) std::string(CHOMP_SHOW_VARS(__VA_ARGS__)) + " = " + debugger_to_string(std::make_tuple(__VA_ARGS__))