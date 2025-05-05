#pragma once

#include <atomic>
#include <cstdint>

namespace VorGen {

extern ::std::atomic<unsigned int> completedCells;
enum Order { Increasing, Decreasing };

}