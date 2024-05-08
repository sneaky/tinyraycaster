#ifndef MAP_H
#define MAP_H

#include <cstdlib>

typedef struct Map {
	size_t w, h;
	Map();
	int get(const size_t i, const size_t j);
	bool is_empty(const size_t i, const size_t j);
} Map;

#endif
