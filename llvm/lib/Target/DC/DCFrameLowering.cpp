#include "DCFrameLowering.h"
#include "DCSubtarget.h"
#include "MCTargetDesc/DCMCTargetDesc.h"
#include "llvm/CodeGen/MachineFrameInfo.h"

using namespace llvm;

DCFrameLowering::DCFrameLowering(const DCSubtarget &STI)
    : TargetFrameLowering(StackGrowsDown, Align(STI.is64Bit() ? 8 : 4), 0),
      STI(STI) {}

void DCFrameLowering::emitPrologue(MachineFunction &MF,
                                   MachineBasicBlock &MBB) const {
  Register ResultReg = STI.getRegisterInfo()->adjustReg(
      MBB, MBB.begin(), DebugLoc(), DC::R2, DC::R2,
      -MF.getFrameInfo().getStackSize(), MachineInstr::FrameSetup);

  assert(ResultReg == DC::R2);
}

void DCFrameLowering::emitEpilogue(MachineFunction &MF,
                                   MachineBasicBlock &MBB) const {
  Register ResultReg = STI.getRegisterInfo()->adjustReg(
      MBB, MBB.getLastNonDebugInstr(), DebugLoc(), DC::R2, DC::R2,
      MF.getFrameInfo().getStackSize(), MachineInstr::FrameDestroy);

  assert(ResultReg == DC::R2);
}

MachineBasicBlock::iterator DCFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MI) const {
  assert(hasReservedCallFrame(MF));

  return MBB.erase(MI);
}

bool DCFrameLowering::hasFPImpl(const MachineFunction &MF) const {
  return false;
}
