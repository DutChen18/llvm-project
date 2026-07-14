#include "DCInstrInfo.h"
#include "DCSubtarget.h"
#include "MCTargetDesc/DCMCTargetDesc.h"
#include "MCTargetDesc/DCMatInt.h"

#define GET_INSTRINFO_TARGET_DESC
#define GET_INSTRINFO_CTOR_DTOR
#include "DCGenInstrInfo.inc"

using namespace llvm;

static unsigned getInverseBranchOpcode(unsigned Opcode) {
  switch (Opcode) {
  default:
    llvm_unreachable("unexpected branch opcode");
  case DC::BEQ:
    return DC::BNE;
  case DC::BNE:
    return DC::BEQ;
  case DC::BLT:
    return DC::BGE;
  case DC::BGE:
    return DC::BLT;
  case DC::BLTU:
    return DC::BGEU;
  case DC::BGEU:
    return DC::BLTU;
  }
}

DCInstrInfo::DCInstrInfo(const DCSubtarget &STI)
    : DCGenInstrInfo(STI, RegInfo, DC::ADJCALLSTACKDOWN, DC::ADJCALLSTACKUP),
      RegInfo(STI.getHwMode()), STI(STI) {}

void DCInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MBBI,
                              const DebugLoc &DL, Register DestReg,
                              Register SrcReg, bool KillSrc, bool RenamableDest,
                              bool RenamableSrc) const {
  BuildMI(MBB, MBBI, DL, get(STI.getADDI()), DestReg)
      .addImm(0)
      .addReg(SrcReg,
              getKillRegState(KillSrc) | getRenamableRegState(RenamableSrc));
}

void DCInstrInfo::storeRegToStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI, Register SrcReg,
    bool IsKill, int FrameIndex, const TargetRegisterClass *RC, Register VReg,
    MachineInstr::MIFlag Flags) const {
  BuildMI(MBB, MBBI, DebugLoc(), get(STI.is64Bit() ? DC::SD : DC::SW))
      .addReg(SrcReg, getKillRegState(IsKill))
      .addImm(0)
      .addFrameIndex(FrameIndex)
      .setMIFlag(Flags);
}

void DCInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator MBBI,
                                       Register DestReg, int FrameIndex,
                                       const TargetRegisterClass *RC,
                                       Register VReg, unsigned SubReg,
                                       MachineInstr::MIFlag Flags) const {
  BuildMI(MBB, MBBI, DebugLoc(), get(STI.is64Bit() ? DC::LD : DC::LW), DestReg)
      .addImm(0)
      .addFrameIndex(FrameIndex)
      .setMIFlag(Flags);
}

bool DCInstrInfo::expandPostRAPseudo(MachineInstr &MI) const {
  MachineBasicBlock &MBB = *MI.getParent();
  DebugLoc DL = MI.getDebugLoc();

  switch (MI.getOpcode()) {
  case DC::PseudoCALL: {
    MachineOperand &Callee = MI.getOperand(0);

    BuildMI(MBB, MI, DL, get(DC::LUI), DC::R1).add(Callee);
    BuildMI(MBB, MI, DL, get(DC::JALR))
        .addReg(DC::R1, RegState::Define | RegState::Dead)
        .addReg(DC::R1, RegState::Kill)
        .add(Callee);

    MBB.erase(MI);

    return true;
  }
  }

  return false;
}

MachineBasicBlock *
DCInstrInfo::getBranchDestBlock(const MachineInstr &MI) const {
  assert(MI.getDesc().isBranch());
  return MI.getOperand(MI.getNumExplicitOperands() - 1).getMBB();
}

bool DCInstrInfo::analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                                MachineBasicBlock *&FBB,
                                SmallVectorImpl<MachineOperand> &Cond,
                                bool AllowModify) const {
  MachineBasicBlock::reverse_iterator I =
      MBB.getLastNonDebugInstr().getReverse();

  if (AllowModify) {
    for (auto J = I; J != MBB.rend() && isUnpredicatedTerminator(*J); J++)
      if (J->getDesc().isUnconditionalBranch() ||
          J->getDesc().isIndirectBranch())
        I = J;

    MBB.erase(std::next(I.getReverse()), MBB.end());
  }

  if (I != MBB.rend() && I->getDesc().isUnconditionalBranch()) {
    TBB = getBranchDestBlock(*I);
    I++;
  }

  if (I != MBB.rend() && I->getDesc().isConditionalBranch()) {
    FBB = TBB;
    TBB = getBranchDestBlock(*I);
    Cond.push_back(MachineOperand::CreateImm(I->getOpcode()));
    Cond.push_back(I->getOperand(0));
    Cond.push_back(I->getOperand(1));
    I++;
  }

  return I != MBB.rend() && isUnpredicatedTerminator(*I);
}

unsigned DCInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                   int *BytesRemoved) const {
  unsigned NumRemoved = 0;
  int TotalBytesRemoved = 0;

  for (auto I = MBB.getLastNonDebugInstr().getReverse();
       I != MBB.rend() && isUnpredicatedTerminator(*I); I++) {
    NumRemoved++;
    TotalBytesRemoved += getInstSizeInBytes(*I);
    I = I->eraseFromParent().getReverse();
  }

  if (BytesRemoved)
    *BytesRemoved = TotalBytesRemoved;

  return NumRemoved;
}

unsigned DCInstrInfo::insertBranch(MachineBasicBlock &MBB,
                                   MachineBasicBlock *TBB,
                                   MachineBasicBlock *FBB,
                                   ArrayRef<MachineOperand> Cond,
                                   const DebugLoc &DL, int *BytesAdded) const {
  unsigned NumAdded = 0;
  int TotalBytesAdded = 0;

  if (!Cond.empty()) {
    NumAdded++;
    TotalBytesAdded +=
        getInstSizeInBytes(*BuildMI(&MBB, DL, get(Cond[0].getImm()))
                                .add(Cond[1])
                                .add(Cond[2])
                                .addMBB(TBB));
    TBB = FBB;
  }

  if (TBB) {
    NumAdded++;
    TotalBytesAdded +=
        getInstSizeInBytes(*BuildMI(&MBB, DL, get(DC::PseudoBR)).addMBB(TBB));
  }

  if (BytesAdded)
    *BytesAdded = TotalBytesAdded;

  return NumAdded;
}

bool DCInstrInfo::reverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  assert(Cond.size() == 3);
  Cond[0].setImm(getInverseBranchOpcode(Cond[0].getImm()));
  return false;
}

const DCRegisterInfo &DCInstrInfo::getRegisterInfo() const { return RegInfo; }

Register DCInstrInfo::movImm(MachineBasicBlock &MBB,
                             MachineBasicBlock::iterator II, const DebugLoc &DL,
                             Register DestReg, int64_t Val,
                             MachineInstr::MIFlag Flag,
                             int64_t *Residual) const {
  Register SrcReg = DC::R0;

  for (const DCMatInt::Inst &Inst :
       DCMatInt::generateInstSeq(Val, STI, Residual)) {
    switch (Inst.getInstKind()) {
    case DCMatInt::Imm:
      BuildMI(MBB, II, DL, get(Inst.getOpcode()), DestReg)
          .addImm(Inst.getImm())
          .setMIFlag(Flag);
      break;
    case DCMatInt::ImmReg:
      BuildMI(MBB, II, DL, get(Inst.getOpcode()), DestReg)
          .addImm(Inst.getImm())
          .addReg(SrcReg, getKillRegState(SrcReg == DestReg))
          .setMIFlag(Flag);
      break;
    case DCMatInt::RegImm:
      BuildMI(MBB, II, DL, get(Inst.getOpcode()), DestReg)
          .addReg(SrcReg, getKillRegState(SrcReg == DestReg))
          .addImm(Inst.getImm())
          .setMIFlag(Flag);
      break;
    }

    SrcReg = DestReg;
  }

  return SrcReg;
}
