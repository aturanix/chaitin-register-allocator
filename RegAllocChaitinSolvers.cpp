#include "RegAllocChaitinSolvers.h"

#include <algorithm>
#include <queue>
#include <vector>

namespace {
std::optional<unsigned int>
findUnusedColor(const alihan::InterferenceGraph &graph,
                const alihan::SolutionMap &solution, unsigned int node) {
  std::vector<char> colorUsage(graph.getFullPhysCount());
  for (auto it = graph.beginEdge(node), e = graph.endEdge(node); it != e;
       ++it) {
    auto itSolution = solution.find(*it);
    if (itSolution != solution.end()) {
      colorUsage[itSolution->second] = true;
    }
  }

  for (unsigned i{graph.getPhysBeginIdx()}, e{graph.getPhysEndIdx()}; i != e;
       ++i) {
    if (!colorUsage[i]) {
      return std::make_optional(i);
    }
  }
  return std::nullopt;
}
} // namespace

namespace alihan {
SolutionMap solveGreedy(const InterferenceGraph &graph) {
  auto comp = [&](unsigned const &virt1, unsigned const &virt2) {
    return graph.compareVirtLess(virt1, virt2);
  };

  std::priority_queue<unsigned, std::vector<unsigned>, decltype(comp)> virts(
      comp);

  for (unsigned i{graph.getVirtBeginIdx()}, end{graph.getVirtEndIdx()};
       i != end; ++i) {
    virts.push(i);
  }

  SolutionMap solution;
  for (unsigned i{graph.getPhysBeginIdx()}, end{graph.getPhysEndIdx()};
       i != end; ++i) {
    solution.insert({i, i});
  }

  while (!virts.empty()) {
    unsigned virt = virts.top();
    virts.pop();

    std::optional<unsigned> color = findUnusedColor(graph, solution, virt);
    if (color.has_value()) {
      solution.insert({virt, color.value()});
    }
  }

  return solution;
}

SolutionMap solveChaitin(const InterferenceGraph &graph) {
  InterferenceGraph tempGraph = graph;

  auto comp = [&](unsigned const &virt1, unsigned const &virt2) {
    return graph.compareVirtLess(virt1, virt2);
  };

  std::vector<unsigned> stack;
  for (std::vector<unsigned> toRemove; tempGraph.getVirtCount() > 0;
       toRemove.clear()) {
    for (auto it = tempGraph.beginVirtNode(), e = tempGraph.endVirtNode();
         it != e; ++it) {
      if (tempGraph.getEdgeCount(*it).value() < tempGraph.getFullPhysCount()) {
        toRemove.push_back(*it);
      }
    }

    for (unsigned virt : toRemove) {
      stack.push_back(virt);
      tempGraph.removeNode(virt);
    }

    if (toRemove.empty() && tempGraph.getVirtCount() > 0) {
      unsigned minVirt = *std::min_element(tempGraph.beginVirtNode(),
                                           tempGraph.endVirtNode(), comp);
      tempGraph.removeNode(minVirt);
    }
  }

  SolutionMap solution;
  for (unsigned i = graph.getPhysBeginIdx(), e = graph.getPhysEndIdx(); i != e;
       ++i) {
    solution.insert({i, i});
  }

  while (!stack.empty()) {
    unsigned virt = stack.back();
    stack.pop_back();
    tempGraph.addNode(virt);
    for (auto it = graph.beginEdge(virt), e = graph.endEdge(virt); it != e;
         ++it) {
      if (tempGraph.hasNode(*it)) {
        tempGraph.addEdge(virt, *it);
      }
    }
    unsigned color = findUnusedColor(tempGraph, solution, virt).value();
    solution.insert({virt, color});
  }
  return solution;
}
} // namespace alihan
