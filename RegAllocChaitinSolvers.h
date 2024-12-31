#pragma once

#include "RegAllocChaitinGraph.h"
#include "RegAllocChaitinRegisters.h"

namespace alihan {
[[nodiscard]] auto solveGreedy(const InterferenceGraph &graph,
                               std::size_t numberOfColors) -> SolutionMap;
[[nodiscard]] auto solveChaitin(const InterferenceGraph &graph,
                                std::size_t numberOfColors) -> SolutionMap;
} // namespace alihan
