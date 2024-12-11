#pragma once

#include <cassert>
#include <optional>
#include <ostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace alihan {
class InterferenceGraph {
private:
  using node_iterator =
      std::unordered_map<unsigned, std::unordered_set<unsigned>>::iterator;
  using const_node_iterator =
      std::unordered_map<unsigned,
                         std::unordered_set<unsigned>>::const_iterator;
  using edge_iterator = std::unordered_set<unsigned>::iterator;
  using const_edge_iterator = std::unordered_set<unsigned>::const_iterator;

public:
  using EdgeIterator = const_edge_iterator;

  class NodeIterator {
  public:
    using value_type = unsigned;

    NodeIterator(const_node_iterator node);
    NodeIterator &operator++();
    unsigned operator*() const;
    bool operator==(NodeIterator const &it) const;
    bool operator!=(NodeIterator const &it) const;

  private:
    const_node_iterator mNode;
  };

  class VirtNodeIterator {
  public:
    using value_type = unsigned;

    VirtNodeIterator(InterferenceGraph const &graph, NodeIterator node,
                     NodeIterator end_node);
    VirtNodeIterator &operator++();
    unsigned operator*();
    bool operator==(VirtNodeIterator const &it);
    bool operator!=(VirtNodeIterator const &it);

  private:
    NodeIterator mNode;
    NodeIterator mEndNode;
    InterferenceGraph const *mGraph;
  };

  class VirtEdgeIterator {
  public:
    using value_type = unsigned;

    VirtEdgeIterator(InterferenceGraph const &graph, EdgeIterator edge,
                     EdgeIterator endEdge);
    VirtEdgeIterator &operator++();
    unsigned operator*();
    bool operator==(VirtEdgeIterator const &it);
    bool operator!=(VirtEdgeIterator const &it);

  private:
    EdgeIterator mEdge;
    EdgeIterator mEndEdge;
    InterferenceGraph const *mGraph;
  };

  static bool compareVirtLess(double weight1, bool spillable1, double weight2,
                              bool spillable2);

  InterferenceGraph() = delete;
  InterferenceGraph(unsigned fullPhysCount, unsigned fullVirtCount);

  unsigned getSize() const;
  unsigned getFullPhysCount() const;
  unsigned getFullVirtCount() const;
  unsigned getPhysCount() const;
  unsigned getVirtCount() const;

  unsigned getPhysBeginIdx() const;
  unsigned getPhysEndIdx() const;
  unsigned getVirtBeginIdx() const;
  unsigned getVirtEndIdx() const;

  unsigned isPhys(unsigned n) const;
  unsigned isVirt(unsigned n) const;

  void setWeight(unsigned node, double weight);
  void setSpillable(unsigned node, bool spillable);

  double getWeight(unsigned node) const;
  bool getSpillable(unsigned node) const;

  bool compareVirtLess(unsigned node1, unsigned node2) const;

  bool hasNode(unsigned node) const;
  bool hasEdge(unsigned node1, unsigned node2) const;
  void addNode(unsigned id);
  bool addEdge(unsigned node1, unsigned node2);
  void removeNode(unsigned node);
  bool removeEdge(unsigned node1, unsigned node2);

  std::optional<unsigned> getEdgeCount(unsigned node) const;
  NodeIterator beginNode() const;
  NodeIterator endNode() const;
  VirtNodeIterator beginVirtNode() const;
  VirtNodeIterator endVirtNode() const;
  EdgeIterator beginEdge(unsigned node) const;
  EdgeIterator endEdge(unsigned node) const;
  VirtEdgeIterator beginVirtEdge(unsigned node) const;
  VirtEdgeIterator endVirtEdge(unsigned node) const;

  std::ostream &print(std::ostream &os) const;

private:
  unsigned mPhysCount{0};
  unsigned mVirtCount{0};
  unsigned mFullPhysCount;
  unsigned mFullVirtCount;
  std::vector<double> mWeight;
  std::vector<char> mSpillable;
  std::unordered_map<unsigned, std::unordered_set<unsigned>> mGraph;
};
} // namespace alihan
