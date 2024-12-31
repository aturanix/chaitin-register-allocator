#include "RegAllocChaitinRegisters.h"
#include "RegAllocChaitinGraph.h"

#include <limits>
#include <optional>
#include <ostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace alihan {
void Registers::addVirt(unsigned id,
                        std::unordered_set<unsigned> candidatePhysIds,
                        double weight, bool spillable) {
  VirtualRegister reg;
  reg.weight = weight;
  reg.spillable = spillable;
  reg.candidatePhysRegs = std::move(candidatePhysIds);
  mVirtRegs.insert({id, std::move(reg)});
  mVirtToVirtOrdinal[id] = mVirtOrdinalToVirt.size();
  mVirtOrdinalToVirt.push_back(id);
}

auto Registers::getVirtCount() const -> unsigned { return mVirtRegs.size(); }

auto Registers::getVirtReg(unsigned virtId) const -> const VirtualRegister * {
  auto it = mVirtRegs.find(virtId);
  return (it == mVirtRegs.cend()) ? nullptr : &it->second;
}

auto Registers::getVirtOrdinalId(unsigned virtId) const
    -> std::optional<unsigned> {
  auto it = mVirtToVirtOrdinal.find(virtId);
  if (it == mVirtToVirtOrdinal.cend()) {
    return {};
  }
  return it->second + mGroups.size();
}

auto Registers::getVirtId(unsigned virtOrdinalId) const
    -> std::optional<unsigned> {
  unsigned i{virtOrdinalId - getVirtOrdinalIdFirst()};
  if (i >= mVirtOrdinalToVirt.size()) {
    return {};
  }
  return mVirtOrdinalToVirt[i];
}

auto Registers::getGroupIdFirst() const -> unsigned { return 0; }

auto Registers::getGroupIdLast() const -> unsigned { return getGroupCount(); }

auto Registers::getVirtOrdinalIdFirst() const -> unsigned {
  return getGroupIdLast();
}

auto Registers::getVirtOrdinalIdLast() const -> unsigned {
  return getVirtOrdinalIdFirst() + getVirtCount();
}

auto Registers::addVirtInterference(unsigned virtId1,
                                    unsigned virtId2) -> bool {
  auto itEnd = mVirtRegs.end();
  if (auto it1 = mVirtRegs.find(virtId1); it1 != itEnd) {
    if (auto it2 = mVirtRegs.find(virtId2); it2 != itEnd) {
      it1->second.interferences.insert(virtId2);
      it2->second.interferences.insert(virtId1);
      return true;
    }
  }
  return false;
}

auto Registers::addPhys(unsigned id,
                        const std::vector<unsigned> &subregIds) -> unsigned {
  auto endIt = mPhysToGroupidx.end();
  auto it = mPhysToGroupidx.find(id);
  if (it != endIt) {
    return it->second;
  }

  for (unsigned subregId : subregIds) {
    it = mPhysToGroupidx.find(subregId);
    if (it != endIt) {
      break;
    }
  }

  unsigned groupId;
  if (it == endIt) {
    groupId = mGroups.size();
    mGroups.emplace_back();
  } else {
    groupId = it->second;
  }

  std::unordered_set<unsigned> &group = mGroups[groupId];
  group.insert(id);
  mPhysToGroupidx.insert({id, groupId});
  for (unsigned subregId : subregIds) {
    group.insert(subregId);
    mPhysToGroupidx.insert({subregId, groupId});
  }
  return groupId;
}

auto Registers::getGroupCount() const -> unsigned { return mGroups.size(); }

auto Registers::getPhysGroupId(unsigned physId) const
    -> std::optional<unsigned> {
  auto it = mPhysToGroupidx.find(physId);
  if (it == mPhysToGroupidx.cend()) {
    return {};
  }
  return it->second;
}

auto Registers::getVirtCandPhysInGroup(unsigned virtId, unsigned groupId) const
    -> std::optional<unsigned> {
  VirtualRegister const *virtReg = getVirtReg(virtId);
  if (!virtReg) {
    return {};
  }

  for (unsigned phys_id : virtReg->candidatePhysRegs) {
    if (getPhysGroupId(phys_id) == groupId) {
      return phys_id;
    }
  }
  return {};
}

auto Registers::createInterferenceGraph() const -> InterferenceGraph {
  InterferenceGraph graph;
  for (unsigned group{getGroupIdFirst()}, e{getGroupIdLast()}; group != e;
       ++group) {
    graph.addNode(group, std::numeric_limits<double>::infinity(), false);
    for (unsigned j{0}; j != group; ++j) {
      graph.addEdge(group, j);
    }
  }

  for (unsigned virt{getVirtOrdinalIdFirst()}, e{getVirtOrdinalIdLast()};
       virt != e; ++virt) {
    const VirtualRegister *virtReg = getVirtReg(getVirtId(virt).value());
    graph.addNode(virt, virtReg->weight, virtReg->spillable);

    std::unordered_set<unsigned> candidateGroups;
    for (unsigned candPhysId : virtReg->candidatePhysRegs) {
      candidateGroups.insert(getPhysGroupId(candPhysId).value());
    }

    for (unsigned group{getGroupIdFirst()}, e{getGroupIdLast()}; group != e;
         ++group) {
      if (!candidateGroups.count(group)) {
        graph.addEdge(virt, group);
      }
    }
  }

  for (unsigned virt{getVirtOrdinalIdFirst()}, e{getVirtOrdinalIdLast()};
       virt != e; ++virt) {
    const VirtualRegister *virtReg = getVirtReg(getVirtId(virt).value());
    for (unsigned interference : virtReg->interferences) {
      graph.addEdge(virt, getVirtOrdinalId(interference).value());
    }
  }
  return graph;
}

auto Registers::print(std::ostream &os) const -> std::ostream & {
  printVirt(os) << '\n';
  return printPhys(os);
}

auto Registers::getVirtReg(unsigned int virtId) -> VirtualRegister * {
  auto it = mVirtRegs.find(virtId);
  return (it == mVirtRegs.end()) ? nullptr : &it->second;
}

auto Registers::printVirtOrdinal(std::ostream &os) const -> std::ostream & {
  os << "ORTOVI: {";
  bool first_ordinal{true};
  for (unsigned i{0}; i != mVirtOrdinalToVirt.size(); ++i) {
    if (!first_ordinal) {
      os << ", ";
    }
    unsigned ordinal = i + mGroups.size();
    os << ordinal << " -> " << *getVirtId(ordinal);
    first_ordinal = false;
  }
  return os << '}';
}

auto Registers::printVirt(std::ostream &os) const -> std::ostream & {
  bool firstVirt{true};
  os << '{';
  for (auto [virtId, virtReg] : mVirtRegs) {
    if (!firstVirt) {
      os << ",\n";
    }
    os << virtId << ": {" << "WE: " << virtReg.weight
       << ", IS: " << (virtReg.spillable ? "true" : "false") << ", PH: {";
    bool firstPhys{true};
    for (unsigned physId : virtReg.candidatePhysRegs) {
      if (!firstPhys) {
        os << ", ";
      }
      os << physId;
      firstPhys = false;
    }
    os << "}, IN: {";
    bool firstInterference{true};
    for (unsigned interferenceVirtId : virtReg.interferences) {
      if (!firstInterference) {
        os << ", ";
      }
      os << interferenceVirtId;
      firstInterference = false;
    }
    os << "}}";
    firstVirt = false;
  }
  os << "}\n";
  return printVirtOrdinal(os);
}

auto Registers::printPhys(std::ostream &os) const -> std::ostream & {
  os << "PHGRPS [";
  bool firstGroup{true};
  for (auto const &group : mGroups) {
    if (!firstGroup) {
      os << ", ";
    }
    os << '{';
    bool firstPhys{true};
    for (unsigned phys : group) {
      if (!firstPhys) {
        os << ", ";
      }
      os << phys;
      firstPhys = false;
    }
    os << '}';
    firstGroup = false;
  }
  os << ']';
  return os;
}

auto convertSolutionMapToSolutionMapLLVM(const Registers &registers,
                                         const SolutionMap &solution)
    -> std::optional<SolutionMapLLVM> {
  std::size_t numberOfColors{0};
  std::unordered_map<std::size_t, unsigned> colorToGroupMapping;
  for (unsigned group{registers.getGroupIdFirst()},
       e{registers.getGroupIdLast()};
       group != e; ++group) {
    auto it = solution.find(group);
    if (it == solution.end()) {
      return {};
    }

    std::size_t color{it->second};
    if (colorToGroupMapping.count(color)) {
      return {};
    }

    colorToGroupMapping.insert({it->second, group});
    ++numberOfColors;
  }

  if (numberOfColors > registers.getGroupCount()) {
    return {};
  }

  SolutionMapLLVM solutionLLVM;
  unsigned virtOrdinalIdFirst{registers.getVirtOrdinalIdFirst()};
  unsigned virtOrdinalIdLast{registers.getVirtOrdinalIdLast()};
  for (auto [reg, color] : solution) {
    if (virtOrdinalIdFirst <= reg && reg < virtOrdinalIdLast) {
      if (std::optional<unsigned> virtId = registers.getVirtId(reg)) {
        if (std::optional<unsigned> physId = registers.getVirtCandPhysInGroup(
                *virtId, colorToGroupMapping[color])) {
          solutionLLVM.insert({*virtId, *physId});
          continue;
        }
      }
      return {};
    }
  }
  return solutionLLVM;
}
} // namespace alihan
