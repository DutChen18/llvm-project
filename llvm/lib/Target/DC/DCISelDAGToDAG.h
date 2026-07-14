#ifndef LLVM_LIB_TARGET_DC_DCISELDAGTODAG_H
#define LLVM_LIB_TARGET_DC_DCISELDAGTODAG_H

#include "llvm/CodeGen/SelectionDAGISel.h"

namespace llvm {

class DCSubtarget;
class DCTargetMachine;

class DCDAGToDAGISel : public SelectionDAGISel {
  const DCSubtarget *Subtarget = nullptr;

public:
  explicit DCDAGToDAGISel(DCTargetMachine &TargetMachine,
                          CodeGenOptLevel OptLevel);

  bool runOnMachineFunction(MachineFunction &MF) override;

  void Select(SDNode *Node) override;

  bool SelectAddrRegImm(SDValue Addr, SDValue &Base, SDValue &Offset);

#define GET_DAGISEL_DECL
#include "DCGenDAGISel.inc"
};

class DCDAGToDAGISelLegacy : public SelectionDAGISelLegacy {
public:
  static char ID;

  explicit DCDAGToDAGISelLegacy(DCTargetMachine &TargetMachine,
                                CodeGenOptLevel OptLevel);
};

} // namespace llvm

#endif
