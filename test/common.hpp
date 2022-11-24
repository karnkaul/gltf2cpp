#pragma once
#include <cstdio>
#include <cstdlib>

#define EXPECT(pred)                                                                                                                                           \
	do { ::test::do_expect(pred, #pred, __FILE__, __func__, __LINE__); } while (false);

#define ASSERT(pred)                                                                                                                                           \
	do { ::test::do_assert(pred, #pred, __FILE__, __func__, __LINE__); } while (false);

namespace test {
struct Bailout {};

inline bool g_error{false};

inline void print_error(char const* type, char const* expr, char const* file, char const* func, int line) {
	std::fprintf(stderr, "%s failed: %s in %s (%s|%d)\n", type, expr, func, file, line);
	g_error = true;
}

inline void do_expect(bool pred, char const* expr, char const* file, char const* func, int line) {
	if (!pred) { print_error("expectation", expr, file, func, line); }
}

inline void do_assert(bool pred, char const* expr, char const* file, char const* func, int line) {
	if (!pred) {
		print_error("assertion", expr, file, func, line);
		throw Bailout{};
	}
}

inline int result() {
	if (g_error) {
		std::fprintf(stderr, "\ntest failed\n");
		return EXIT_FAILURE;
	}
	std::fprintf(stdout, "test succeeded\n");
	return EXIT_SUCCESS;
}
} // namespace test
