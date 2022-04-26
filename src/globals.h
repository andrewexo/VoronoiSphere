#ifndef GLOBALS_H
#define GLOBALS_H

#include <atomic>

extern std::atomic<unsigned int> completedCells;

enum Order { Increasing, Decreasing };

#endif