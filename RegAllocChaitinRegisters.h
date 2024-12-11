#pragma once

#include "RegAllocChaitinGraph.h"

#include <optional>
#include <ostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace alihan {
using SolutionMap = std::unordered_map<unsigned, unsigned>;

struct SolutionMapLLVM {
public:
  using assign_iterator_const =
      std::unordered_map<unsigned, unsigned>::const_iterator;
  using spill_iterator_const = std::vector<unsigned>::const_iterator;

  SolutionMapLLVM() = default;

  void assign(unsigned virt, unsigned phys);
  void spill(unsigned virt);
  assign_iterator_const beginAssignments() const;
  assign_iterator_const endAssignments() const;
  spill_iterator_const beginSpills() const;
  spill_iterator_const endSpills() const;

  std::ostream &print(std::ostream &os) const;

private:
  std::unordered_map<unsigned, unsigned> mAssingments;
  std::vector<unsigned> mSpills;
};

class __attribute__((visibility("hidden"))) Registers {
public:
  struct VirtualRegister {
    double weight;
    bool spillable;
    std::unordered_set<unsigned> interferences;
    std::unordered_set<unsigned> candidatePhysRegs;
  };

  void addVirt(unsigned id, std::unordered_set<unsigned> candidatePhysIds,
               double weight, bool spillable);
  unsigned virtCount() const;
  const VirtualRegister *getVirtReg(unsigned virtId) const;
  std::optional<unsigned> getVirtOrdinalId(unsigned virtId) const;
  std::optional<unsigned> getVirtId(unsigned virtOrdinalId) const;
  unsigned getGroupBeginId() const;
  unsigned getGroupEndId() const;
  unsigned getVirtOrdinalBeginId() const;
  unsigned getVirtOrdinalEndId() const;
  bool addVirtInterference(unsigned virtId1, unsigned virtId2);
  unsigned addPhys(unsigned id, std::vector<unsigned> const &subregIds);
  unsigned getGroupCount() const;
  std::optional<unsigned> getPhysGroupId(unsigned physId) const;
  std::optional<unsigned> getVirtCandPhysInGroup(unsigned virtId,
                                                 unsigned groupId) const;
  InterferenceGraph createInterferenceGraph() const;
  std::ostream &print(std::ostream &os) const;

private:
  VirtualRegister *getVirtReg(unsigned virtId);
  std::ostream &printVirtOrdinal(std::ostream &os) const;
  std::ostream &printVirt(std::ostream &os) const;
  std::ostream &printPhys(std::ostream &os) const;

  std::unordered_map<unsigned, VirtualRegister> mVirtRegs;
  std::unordered_map<unsigned, unsigned> mVirtToVirtOrdinal;
  std::vector<unsigned> mVirtOrdinalToVirt;
  std::unordered_map<unsigned, unsigned> mPhysToGroupidx;
  std::vector<std::unordered_set<unsigned>> mGroups;
};

SolutionMapLLVM
convertSolutionMapToSolutionMapLLVM(Registers const &registers,
                                    SolutionMap const &solution);
} // namespace alihan
