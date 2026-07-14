#include "DCRegisterInfo.h"
#include "DCSubtarget.h"
#include "MCTargetDesc/DCMCTargetDesc.h"

#define GET_REGINFO_TARGET_DESC
#include "DCGenRegisterInfo.inc"

using namespace llvm;

DCRegisterInfo::DCRegisterInfo(unsigned HwMode)
    : DCGenRegisterInfo(DC::R1, 0, 0, 0, HwMode) {}

const MCPhysReg *
DCRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_SaveList;
}

const uint32_t *DCRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                                     CallingConv::ID CC) const {
  return CSR_RegMask;
}

BitVector DCRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  markSuperRegs(Reserved, DC::R0);
  markSuperRegs(Reserved, DC::R2);
  markSuperRegs(Reserved, DC::R3);

  return Reserved;
}

bool DCRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator MI,
                                         int SPAdj, unsigned int FIOperandNum,
                                         RegScavenger *RS) const {
  assert(SPAdj == 0);

  MachineFunction &MF = *MI->getParent()->getParent();
  const DCSubtarget &ST = MF.getSubtarget<DCSubtarget>();

  Register FrameReg;
  StackOffset Offset = getFrameLowering(MF)->getFrameIndexReference(
      MF, MI->getOperand(FIOperandNum).getIndex(), FrameReg);

  assert(Offset.getScalable() == 0);

  int64_t Val = Offset.getFixed() + MI->getOperand(FIOperandNum - 1).getImm();
  int64_t Residual;
  Register DestReg;

  if (ST.is32Bit())
    Val = SignExtend64<32>(Val);

  if (MI->getOpcode() == ST.getADDI())
    DestReg = MI->getOperand(0).getReg();
  else
    DestReg = MF.getRegInfo().createVirtualRegister(&DC::GPRegClass);

  Register ResultReg =
      adjustReg(*MI->getParent(), MI, MI->getDebugLoc(), DestReg, FrameReg, Val,
                MachineInstr::NoFlags, &Residual);

  if (MI->getOpcode() == ST.getADDI() && ResultReg == DestReg &&
      Residual == 0) {
    MI->eraseFromParent();
    return true;
  }

  MI->getOperand(FIOperandNum)
      .ChangeToRegister(ResultReg, false, false, ResultReg == DestReg);
  MI->getOperand(FIOperandNum - 1).ChangeToImmediate(Residual);
  return false;
}

Register DCRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return DC::R2;
}

bool DCRegisterInfo::requiresRegisterScavenging(
    const MachineFunction &MF) const {
  return true;
}

bool DCRegisterInfo::requiresFrameIndexScavenging(
    const MachineFunction &MF) const {
  return true;
}

Register DCRegisterInfo::adjustReg(MachineBasicBlock &MBB,
                                   MachineBasicBlock::iterator II,
                                   const DebugLoc &DL, Register DestReg,
                                   Register SrcReg, int64_t Val,
                                   MachineInstr::MIFlag Flag,
                                   int64_t *Residual) const {
  MachineFunction &MF = *MBB.getParent();
  const DCSubtarget &ST = MF.getSubtarget<DCSubtarget>();
  const DCInstrInfo *TII = ST.getInstrInfo();

  if (Val >= -4096 && Val <= 4094) {
    if (!isInt<12>(Val)) {
      int64_t Adj = Val < 0 ? -2048 : 2047;

      BuildMI(MBB, II, DL, TII->get(ST.getADDI()), DestReg)
          .addImm(Adj)
          .addReg(SrcReg, getKillRegState(SrcReg == DestReg))
          .setMIFlag(Flag);

      SrcReg = DestReg;
      Val -= Adj;
    }

    if (Residual)
      *Residual = Val;
    else if (Val != 0) {
      BuildMI(MBB, II, DL, TII->get(ST.getADDI()), DestReg)
          .addImm(Val)
          .addReg(SrcReg, getKillRegState(SrcReg == DestReg))
          .setMIFlag(Flag);

      SrcReg = DestReg;
    }
  } else {
    Register ScratchReg =
        MF.getRegInfo().createVirtualRegister(&DC::GPRegClass);
    Register ResultReg =
        TII->movImm(MBB, II, DL, ScratchReg, Val, Flag, Residual);

    BuildMI(MBB, II, DL, TII->get(ST.getADD()), DestReg)
        .addReg(ResultReg, getKillRegState(ResultReg == ScratchReg))
        .addReg(SrcReg, getKillRegState(SrcReg == DestReg))
        .setMIFlag(Flag);

    SrcReg = DestReg;
  }

  return SrcReg;
}
