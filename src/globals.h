#ifndef GLOBALS_H
#define GLOBALS_H

#include <atomic>

namespace VorGen {

extern ::std::atomic<unsigned int> completedCells;
enum Order { Increasing, Decreasing };

}

#endif