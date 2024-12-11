#include "RegAllocChaitinRegisters.h"
#include "RegAllocChaitinGraph.h"

#include <optional>
#include <ostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace alihan {
void SolutionMapLLVM::assign(unsigned int virt, unsigned int phys) {
  mAssingments.insert({virt, phys});
}

void SolutionMapLLVM::spill(unsigned int virt) { mSpills.push_back(virt); }

SolutionMapLLVM::assign_iterator_const
SolutionMapLLVM::beginAssignments() const {
  return mAssingments.cbegin();
}

SolutionMapLLVM::assign_iterator_const SolutionMapLLVM::endAssignments() const {
  return mAssingments.cend();
}

SolutionMapLLVM::spill_iterator_const SolutionMapLLVM::beginSpills() const {
  return mSpills.cbegin();
}

SolutionMapLLVM::spill_iterator_const SolutionMapLLVM::endSpills() const {
  return mSpills.cend();
}

std::ostream &SolutionMapLLVM::print(std::ostream &os) const {
  os << "ASSIGN: {";
  bool firstAssignment{true};
  for (auto [k, v] : mAssingments) {
    if (!firstAssignment) {
      os << ", ";
    }
    os << k << ": " << v;
    firstAssignment = false;
  }
  os << "}\n";

  os << "SPILLS: {";
  bool firstSpill{true};
  for (unsigned spill : mSpills) {
    if (!firstSpill) {
      os << ", ";
    }
    os << spill;
    firstSpill = false;
  }
  return os << '}';
}

void Registers::addVirt(unsigned int id,
                        std::unordered_set<unsigned int> candidatePhysIds,
                        double weight, bool spillable) {
  VirtualRegister reg;
  reg.weight = weight;
  reg.spillable = spillable;
  reg.candidatePhysRegs = std::move(candidatePhysIds);
  mVirtRegs.insert({id, std::move(reg)});
  mVirtToVirtOrdinal[id] = mVirtOrdinalToVirt.size();
  mVirtOrdinalToVirt.push_back(id);
}

unsigned Registers::virtCount() const { return mVirtRegs.size(); }

const Registers::VirtualRegister *
Registers::getVirtReg(unsigned int virtId) const {
  auto it = mVirtRegs.find(virtId);
  return (it == mVirtRegs.cend()) ? nullptr : &it->second;
}

std::optional<unsigned int>
Registers::getVirtOrdinalId(unsigned int virtId) const {
  auto it = mVirtToVirtOrdinal.find(virtId);
  return (it == mVirtToVirtOrdinal.cend())
             ? std::nullopt
             : std::make_optional(it->second + mGroups.size());
}

std::optional<unsigned int>
Registers::getVirtId(unsigned int virtOrdinalId) const {
  unsigned i = virtOrdinalId - mGroups.size();
  return (i < mVirtOrdinalToVirt.size())
             ? std::make_optional(mVirtOrdinalToVirt[i])
             : std::nullopt;
}

unsigned int Registers::getGroupBeginId() const { return 0; }

unsigned int Registers::getGroupEndId() const {
  return getGroupBeginId() + getGroupCount();
}

unsigned int Registers::getVirtOrdinalBeginId() const {
  return getGroupEndId();
}

unsigned int Registers::getVirtOrdinalEndId() const {
  return getVirtOrdinalBeginId() + virtCount();
}

bool Registers::addVirtInterference(unsigned int virtId1,
                                    unsigned int virtId2) {
  auto itEnd = mVirtRegs.end();
  auto it1 = mVirtRegs.find(virtId1);
  if (it1 == itEnd) {
    return false;
  }

  auto it2 = mVirtRegs.find(virtId2);
  if (it2 == itEnd) {
    return false;
  }

  it1->second.interferences.insert(virtId2);
  it2->second.interferences.insert(virtId1);
  return true;
}

unsigned int Registers::addPhys(unsigned int id,
                                const std::vector<unsigned int> &subregIds) {
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

unsigned int Registers::getGroupCount() const { return mGroups.size(); }

std::optional<unsigned int>
Registers::getPhysGroupId(unsigned int physId) const {
  auto it = mPhysToGroupidx.find(physId);
  return (it == mPhysToGroupidx.cend()) ? std::nullopt
                                        : std::make_optional(it->second);
}

std::optional<unsigned int>
Registers::getVirtCandPhysInGroup(unsigned int virtId,
                                  unsigned int groupId) const {
  VirtualRegister const *virtReg = getVirtReg(virtId);
  if (!virtReg) {
    return std::nullopt;
  }

  for (unsigned phys_id : virtReg->candidatePhysRegs) {
    if (getPhysGroupId(phys_id) == groupId) {
      return std::make_optional(phys_id);
    }
  }
  return std::nullopt;
}

InterferenceGraph Registers::createInterferenceGraph() const {
  unsigned groupCount = mGroups.size();
  unsigned virtCount = mVirtRegs.size();
  InterferenceGraph graph(groupCount, virtCount);

  for (unsigned i = getGroupBeginId(), e = getGroupEndId(); i != e; ++i) {
    graph.addNode(i);
  }

  for (unsigned i = getVirtOrdinalBeginId(), e = getVirtOrdinalEndId(); i != e;
       ++i) {
    graph.addNode(i);
  }

  for (unsigned virt = getVirtOrdinalBeginId(), e = getVirtOrdinalEndId();
       virt != e; ++virt) {
    Registers::VirtualRegister const *virtReg =
        getVirtReg(getVirtId(virt).value());
    graph.setSpillable(virt, virtReg->spillable);
    graph.setWeight(virt, virtReg->weight);
    for (unsigned interference : virtReg->interferences) {
      graph.addEdge(virt, getVirtOrdinalId(interference).value());
    }

    std::unordered_set<unsigned> candidateGroups;
    for (unsigned candPhysId : virtReg->candidatePhysRegs) {
      candidateGroups.insert(getPhysGroupId(candPhysId).value());
    }

    for (unsigned group = getGroupBeginId(), e = getGroupEndId(); group != e;
         ++group) {
      if (candidateGroups.find(group) == candidateGroups.end()) {
        graph.addEdge(virt, group);
      }
    }
  }

  return graph;
}

std::ostream &Registers::print(std::ostream &os) const {
  printVirt(os) << '\n';
  return printPhys(os);
}

Registers::VirtualRegister *Registers::getVirtReg(unsigned int virtId) {
  auto it = mVirtRegs.find(virtId);
  return (it == mVirtRegs.end()) ? nullptr : &it->second;
}

std::ostream &Registers::printVirtOrdinal(std::ostream &os) const {
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

std::ostream &Registers::printVirt(std::ostream &os) const {
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

std::ostream &Registers::printPhys(std::ostream &os) const {
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

SolutionMapLLVM
convertSolutionMapToSolutionMapLLVM(const Registers &registers,
                                    const SolutionMap &solution) {
  SolutionMapLLVM solutionLLVM;
  std::vector<unsigned> spills;
  for (unsigned i = registers.getVirtOrdinalBeginId(),
                end = registers.getVirtOrdinalEndId();
       i != end; ++i) {
    unsigned virtId = registers.getVirtId(i).value();
    auto it = solution.find(i);
    if (it == solution.end()) {
      solutionLLVM.spill(virtId);
    } else {
      unsigned physId =
          registers.getVirtCandPhysInGroup(virtId, it->second).value();
      solutionLLVM.assign(virtId, physId);
    }
  }
  return solutionLLVM;
}
} // namespace alihan
