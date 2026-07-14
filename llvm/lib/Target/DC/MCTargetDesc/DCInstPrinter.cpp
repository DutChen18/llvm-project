#include "DCInstPrinter.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"

using namespace llvm;

#include "DCGenAsmWriter.inc"

DCInstPrinter::DCInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                             const MCRegisterInfo &MRI)
    : MCInstPrinter(MAI, MII, MRI) {}

void DCInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                              StringRef Annot, const MCSubtargetInfo &STI,
                              raw_ostream &OS) {
  printInstruction(MI, Address, OS);
}

void DCInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                 raw_ostream &OS) {
  const MCOperand &MO = MI->getOperand(OpNo);

  if (MO.isReg()) {
    markup(OS, Markup::Register) << getRegisterName(MO.getReg());
  } else if (MO.isImm()) {
    markup(OS, Markup::Immediate) << formatDec(MO.getImm());
  } else {
    assert(MO.isExpr());
    MAI.printExpr(OS, *MO.getExpr());
  }
}
