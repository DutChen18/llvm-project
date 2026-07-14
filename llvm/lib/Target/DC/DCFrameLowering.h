#ifndef LLVM_LIB_TARGET_DC_DCFRAMELOWERING_H
#define LLVM_LIB_TARGET_DC_DCFRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {

class DCSubtarget;

class DCFrameLowering : public TargetFrameLowering {
  const DCSubtarget &STI;

public:
  DCFrameLowering(const DCSubtarget &STI);

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI) const override;

protected:
  bool hasFPImpl(const MachineFunction &MF) const override;
};

} // namespace llvm

#endif
