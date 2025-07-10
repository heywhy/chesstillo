#ifndef ENGINE_CONFIG_HPP
#define ENGINE_CONFIG_HPP

#include <climits>

#define MAX_SCORE (INT_MAX - 2)
#define MIN_SCORE -MAX_SCORE

// #define MAX_DEPTH 10  // 125
#define MAX_DEPTH 10
#define MAX_THREADS 256

#define SPLIT_MIN_DEPTH 6
#define SPLIT_MAX_SLAVES 3
#define SPLIT_MIN_MOVES_TODO 1

#endif
