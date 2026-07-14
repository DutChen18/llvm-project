#ifndef LLVM_LIB_TARGET_DC_MCTARGETDESC_DCINSTPRINTER_H
#define LLVM_LIB_TARGET_DC_MCTARGETDESC_DCINSTPRINTER_H

#include "llvm/MC/MCInstPrinter.h"

namespace llvm {

class DCInstPrinter : public MCInstPrinter {
public:
  DCInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                const MCRegisterInfo &MRI);

  std::pair<const char *, uint64_t>
  getMnemonic(const MCInst &MI) const override;

  void printInst(const MCInst *MI, uint64_t Address, StringRef Annot,
                 const MCSubtargetInfo &STI, raw_ostream &OS) override;

  void printOperand(const MCInst *MI, unsigned OpNo, raw_ostream &OS);

  void printInstruction(const MCInst *MI, uint64_t Address, raw_ostream &OS);

  static const char *getRegisterName(MCRegister Reg);
};

} // namespace llvm

#endif
