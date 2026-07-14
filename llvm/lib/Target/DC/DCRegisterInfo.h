#ifndef LLVM_LIB_TARGET_DC_DCREGISTERINFO_H
#define LLVM_LIB_TARGET_DC_DCREGISTERINFO_H

#define GET_REGINFO_HEADER
#include "DCGenRegisterInfo.inc"

namespace llvm {

class DCRegisterInfo : public DCGenRegisterInfo {
public:
  DCRegisterInfo(unsigned HwMode);

  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;

  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const override;

  BitVector getReservedRegs(const MachineFunction &MF) const override;

  bool eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                           unsigned int FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  Register getFrameRegister(const MachineFunction &MF) const override;

  bool requiresRegisterScavenging(const MachineFunction &MF) const override;

  bool requiresFrameIndexScavenging(const MachineFunction &MF) const override;

  Register adjustReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator II,
                     const DebugLoc &DL, Register DestReg, Register SrcReg,
                     int64_t Val, MachineInstr::MIFlag Flag,
                     int64_t *Residual = nullptr) const;
};

} // namespace llvm

#endif
