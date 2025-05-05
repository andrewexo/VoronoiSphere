#pragma once

#include <atomic>
#include <cstdint>

namespace VorGen {

extern ::std::atomic<std::size_t> completedCells;
enum Order { Increasing, Decreasing };

}