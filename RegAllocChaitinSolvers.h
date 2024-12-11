#pragma once

#include "RegAllocChaitinGraph.h"
#include "RegAllocChaitinRegisters.h"

namespace alihan {
SolutionMap solveGreedy(InterferenceGraph const &graph);
SolutionMap solveChaitin(InterferenceGraph const &graph);
} // namespace alihan
