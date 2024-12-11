#include "RegAllocChaitinGraph.h"

namespace alihan {
InterferenceGraph::NodeIterator::NodeIterator(const_node_iterator node)
    : mNode{node} {}

InterferenceGraph::NodeIterator &InterferenceGraph::NodeIterator::operator++() {
  ++mNode;
  return *this;
}

unsigned int InterferenceGraph::NodeIterator::operator*() const {
  return mNode->first;
}

bool InterferenceGraph::NodeIterator::operator==(const NodeIterator &it) const {
  return mNode == it.mNode;
}

bool InterferenceGraph::NodeIterator::operator!=(const NodeIterator &it) const {
  return !(*this == it);
}

InterferenceGraph::VirtNodeIterator::VirtNodeIterator(
    const InterferenceGraph &graph, NodeIterator node, NodeIterator endNode)
    : mNode{node}, mEndNode{endNode}, mGraph{&graph} {
  if (mNode != mEndNode && !mGraph->isVirt(*mNode)) {
    ++*this;
  }
}

InterferenceGraph::VirtNodeIterator &
InterferenceGraph::VirtNodeIterator::operator++() {
  while (mNode != mEndNode) {
    ++mNode;
    if (mNode != mEndNode && mGraph->isVirt(*mNode)) {
      break;
    }
  }
  return *this;
}

unsigned int InterferenceGraph::VirtNodeIterator::operator*() { return *mNode; }

bool InterferenceGraph::VirtNodeIterator::operator==(
    const VirtNodeIterator &it) {
  return mGraph == it.mGraph && mNode == it.mNode;
}

bool InterferenceGraph::VirtNodeIterator::operator!=(
    const VirtNodeIterator &it) {
  return !(*this == it);
}

InterferenceGraph::VirtEdgeIterator::VirtEdgeIterator(
    const InterferenceGraph &graph, EdgeIterator edge, EdgeIterator endEdge)
    : mEdge{edge}, mEndEdge{endEdge}, mGraph{&graph} {
  if (mEdge != mEndEdge && !mGraph->isVirt(*mEdge)) {
    ++*this;
  }
}

InterferenceGraph::VirtEdgeIterator &
InterferenceGraph::VirtEdgeIterator::operator++() {
  while (mEdge != mEndEdge) {
    ++mEdge;
    if (mEdge != mEndEdge && mGraph->isVirt(*mEdge)) {
      break;
    }
  }
  return *this;
}

unsigned int InterferenceGraph::VirtEdgeIterator::operator*() { return *mEdge; }

bool InterferenceGraph::VirtEdgeIterator::operator==(
    const VirtEdgeIterator &it) {
  return mGraph == it.mGraph && mEdge == it.mEdge;
}

bool InterferenceGraph::VirtEdgeIterator::operator!=(
    const VirtEdgeIterator &it) {
  return !(*this == it);
}

bool InterferenceGraph::compareVirtLess(double weight1, bool spillable1,
                                        double weight2, bool spillable2) {
  if (spillable1 && spillable2) {
    return weight1 < weight2;
  } else {
    return spillable1;
  }
}

InterferenceGraph::InterferenceGraph(unsigned int fullPhysCount,
                                     unsigned int fullVirtCount)
    : mFullPhysCount{fullPhysCount}, mFullVirtCount{fullVirtCount},
      mWeight(fullVirtCount), mSpillable(fullVirtCount) {}

unsigned int InterferenceGraph::getSize() const { return mGraph.size(); }

unsigned int InterferenceGraph::getFullPhysCount() const {
  return mFullPhysCount;
}

unsigned int InterferenceGraph::getFullVirtCount() const {
  return mFullVirtCount;
}

unsigned int InterferenceGraph::getPhysCount() const { return mPhysCount; }

unsigned int InterferenceGraph::getVirtCount() const { return mVirtCount; }

unsigned int InterferenceGraph::getPhysBeginIdx() const { return 0; }

unsigned int InterferenceGraph::getPhysEndIdx() const { return mFullPhysCount; }

unsigned int InterferenceGraph::getVirtBeginIdx() const {
  return mFullPhysCount;
}

unsigned int InterferenceGraph::getVirtEndIdx() const {
  return mFullPhysCount + mFullVirtCount;
}

unsigned int InterferenceGraph::isPhys(unsigned int n) const {
  return getPhysBeginIdx() <= n && n < getPhysEndIdx();
}

unsigned int InterferenceGraph::isVirt(unsigned int n) const {
  return getVirtBeginIdx() <= n && n < getVirtEndIdx();
}

void InterferenceGraph::setWeight(unsigned int node, double weight) {
  mWeight[node - getVirtBeginIdx()] = weight;
}

void InterferenceGraph::setSpillable(unsigned int node, bool spillable) {
  mSpillable[node - getVirtBeginIdx()] = spillable;
}

double InterferenceGraph::getWeight(unsigned int node) const {
  return mWeight[node - getVirtBeginIdx()];
}

bool InterferenceGraph::getSpillable(unsigned int node) const {
  return mSpillable[node - getVirtBeginIdx()];
}

bool InterferenceGraph::compareVirtLess(unsigned int node1,
                                        unsigned int node2) const {
  return InterferenceGraph::compareVirtLess(
      getWeight(node1), getSpillable(node1), getWeight(node2),
      getSpillable(node2));
}

bool InterferenceGraph::hasNode(unsigned int node) const {
  return mGraph.find(node) != mGraph.end();
}

bool InterferenceGraph::hasEdge(unsigned int node1, unsigned int node2) const {
  const_node_iterator it1 = mGraph.find(node1);
  if (it1 == mGraph.end() || !hasNode(node2)) {
    return false;
  }

  return it1->second.find(node2) != it1->second.end();
}

void InterferenceGraph::addNode(unsigned int id) {
  assert(isPhys(id) || isVirt(id));
  mGraph.insert({id, {}});
  if (isPhys(id)) {
    ++mPhysCount;
  } else {
    ++mVirtCount;
  }
}

bool InterferenceGraph::addEdge(unsigned int node1, unsigned int node2) {
  node_iterator end = mGraph.end();
  node_iterator it1 = mGraph.find(node1);
  if (it1 == end) {
    return false;
  }

  node_iterator it2 = mGraph.find(node2);
  if (it2 == end) {
    return false;
  }

  it1->second.insert(node2);
  it2->second.insert(node1);
  return true;
}

void InterferenceGraph::removeNode(unsigned int node) {
  node_iterator it = mGraph.find(node);
  if (it == mGraph.end()) {
    return;
  }

  for (unsigned edgeNode : it->second) {
    mGraph[edgeNode].erase(it->first);
  }
  mGraph.erase(it);
  if (isPhys(node)) {
    --mPhysCount;
  } else {
    --mVirtCount;
  }
}

bool InterferenceGraph::removeEdge(unsigned int node1, unsigned int node2) {
  node_iterator end = mGraph.end();
  node_iterator it1 = mGraph.find(node1);
  if (it1 == end) {
    return false;
  }

  node_iterator it2 = mGraph.find(node2);
  if (it2 == end) {
    return false;
  }

  return it1->second.erase(node2) && it2->second.erase(node1);
}

std::optional<unsigned int>
InterferenceGraph::getEdgeCount(unsigned int node) const {
  auto it = mGraph.find(node);
  return (it == mGraph.end()) ? std::nullopt
                              : std::make_optional<unsigned>(it->second.size());
}

InterferenceGraph::NodeIterator InterferenceGraph::beginNode() const {
  return NodeIterator(mGraph.begin());
}

InterferenceGraph::NodeIterator InterferenceGraph::endNode() const {
  return NodeIterator(mGraph.end());
}

InterferenceGraph::VirtNodeIterator InterferenceGraph::beginVirtNode() const {
  return VirtNodeIterator(*this, beginNode(), endNode());
}

InterferenceGraph::VirtNodeIterator InterferenceGraph::endVirtNode() const {
  return VirtNodeIterator(*this, endNode(), endNode());
}

InterferenceGraph::EdgeIterator
InterferenceGraph::beginEdge(unsigned int node) const {
  return mGraph.find(node)->second.begin();
}

InterferenceGraph::EdgeIterator
InterferenceGraph::endEdge(unsigned int node) const {
  return mGraph.find(node)->second.end();
}

InterferenceGraph::VirtEdgeIterator
InterferenceGraph::beginVirtEdge(unsigned int node) const {
  return VirtEdgeIterator(*this, beginEdge(node), endEdge(node));
}

InterferenceGraph::VirtEdgeIterator
InterferenceGraph::endVirtEdge(unsigned int node) const {
  return VirtEdgeIterator(*this, endEdge(node), endEdge(node));
}

std::ostream &InterferenceGraph::print(std::ostream &os) const {
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
} // namespace alihan
