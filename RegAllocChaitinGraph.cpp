#include "RegAllocChaitinGraph.h"

#include <cstddef>
#include <optional>

namespace alihan {
InterferenceGraph::Node::Node(double weight, bool spillable)
    : mWeight{weight}, mSpillable{spillable} {}

auto InterferenceGraph::Node::getWeight() const -> double { return mWeight; }

auto InterferenceGraph::Node::getSpillable() const -> bool {
  return mSpillable;
}

auto InterferenceGraph::Node::getEdgeCount() const -> std::size_t {
  return mEdges.size();
}

auto InterferenceGraph::Node::hasEdge(unsigned node) const -> bool {
  return mEdges.count(node);
}

void InterferenceGraph::Node::addEdge(unsigned node) { mEdges.insert(node); }

void InterferenceGraph::Node::removeEdge(unsigned node) { mEdges.erase(node); }

auto InterferenceGraph::Node::isLessThan(const Node &other) const -> bool {
  if (mSpillable && other.mSpillable) {
    return mWeight < other.mWeight;
  } else {
    return mSpillable;
  }
}

auto InterferenceGraph::Node::begin() const -> EdgeIterator {
  return mEdges.begin();
}

auto InterferenceGraph::Node::end() const -> EdgeIterator {
  return mEdges.end();
}

InterferenceGraph::NodeIterator::NodeIterator(iterator node) : mNode{node} {}

auto InterferenceGraph::NodeIterator::operator++() -> NodeIterator & {
  ++mNode;
  return *this;
}

auto InterferenceGraph::NodeIterator::operator*() const -> unsigned {
  return mNode->first;
}

auto InterferenceGraph::NodeIterator::operator==(const NodeIterator &it) const
    -> bool {
  return mNode == it.mNode;
}

auto InterferenceGraph::NodeIterator::operator!=(const NodeIterator &it) const
    -> bool {
  return !(*this == it);
}

auto InterferenceGraph::isEmpty() const -> bool { return mGraph.empty(); }

auto InterferenceGraph::getSize() const -> std::size_t { return mGraph.size(); }

auto InterferenceGraph::getWeight(unsigned node) const
    -> std::optional<double> {
  if (const Node *n = getNode(node)) {
    return n->getWeight();
  }
  return {};
}

auto InterferenceGraph::getSpillable(unsigned node) const
    -> std::optional<bool> {
  if (const Node *n = getNode(node)) {
    return n->getSpillable();
  }
  return {};
}

auto InterferenceGraph::getEdgeCount(unsigned node) const
    -> std::optional<std::size_t> {
  if (const Node *n = getNode(node)) {
    return n->getEdgeCount();
  }
  return {};
}

auto InterferenceGraph::hasNode(unsigned node) const -> bool {
  return mGraph.count(node);
}

auto InterferenceGraph::hasEdge(unsigned node1, unsigned node2) const -> bool {
  const Node *n1 = getNode(node1);
  return n1 && n1->hasEdge(node2);
}

void InterferenceGraph::addNode(unsigned id, double weight, bool spillable) {
  mGraph.insert({id, Node(weight, spillable)});
}

auto InterferenceGraph::addEdge(unsigned node1, unsigned node2) -> bool {
  if (Node *n1 = getNode(node1)) {
    if (Node *n2 = getNode(node2)) {
      n1->addEdge(node2);
      n2->addEdge(node1);
      return true;
    }
  }
  return false;
}

void InterferenceGraph::removeNode(unsigned node) {
  if (Node *n = getNode(node)) {
    for (unsigned edge : *n) {
      Node *e = getNode(edge);
      e->removeEdge(node);
    }
    mGraph.erase(node);
  }
}

auto InterferenceGraph::removeEdge(unsigned node1, unsigned node2) -> bool {
  if (Node *n1 = getNode(node1)) {
    if (Node *n2 = getNode(node2)) {
      n1->removeEdge(node2);
      n2->removeEdge(node1);
      return true;
    }
  }
  return false;
}

auto InterferenceGraph::isNodeLessThan(unsigned node1, unsigned node2) const
    -> std::optional<bool> {
  if (const Node *n1 = getNode(node1)) {
    if (const Node *n2 = getNode(node2)) {
      return n1->isLessThan(*n2);
    }
  }
  return {};
}

auto InterferenceGraph::getNodeRange() const -> Range<NodeIterator> {
  return Range<NodeIterator>(mGraph.begin(), mGraph.end());
}

auto InterferenceGraph::getEdgeRange(unsigned node) const
    -> std::optional<Range<EdgeIterator>> {
  if (const Node *n = getNode(node)) {
    return Range<EdgeIterator>(n->begin(), n->end());
  }
  return {};
}

auto InterferenceGraph::print(std::ostream &os) const -> std::ostream & {
  os << '[';
  bool firstNode{true};
  for (auto const &node : mGraph) {
    if (!firstNode) {
      os << ", ";
    }
    firstNode = false;
    os << node.first << ": {";

    bool firstEdge{true};
    for (unsigned edgeNode : node.second) {
      if (!firstEdge) {
        os << ", ";
      }
      firstEdge = false;
      os << edgeNode;
    }

    os << "}";
  }
  os << ']';
  return os;
}

auto InterferenceGraph::getNode(unsigned node) const -> const Node * {
  auto it = mGraph.find(node);
  return (it == mGraph.end()) ? nullptr : &it->second;
}

auto InterferenceGraph::getNode(unsigned node) -> Node * {
  auto it = mGraph.find(node);
  return (it == mGraph.end()) ? nullptr : &it->second;
}
} // namespace alihan
