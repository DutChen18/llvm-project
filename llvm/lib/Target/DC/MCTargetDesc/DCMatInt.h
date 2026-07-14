#ifndef LLVM_LIB_TARGET_DC_MCTARGETDESC_DCMATINT_H
#define LLVM_LIB_TARGET_DC_MCTARGETDESC_DCMATINT_H

#include "llvm/ADT/SmallVector.h"

namespace llvm {

class MCSubtargetInfo;

namespace DCMatInt {

enum InstKind {
  Imm,
  ImmReg,
  RegImm,
};

class Inst {
  unsigned Opcode;
  int64_t Imm;

public:
  Inst(unsigned Opcode, int64_t Imm);

  unsigned getOpcode() const;
  int64_t getImm() const;
  InstKind getInstKind() const;
};

using InstSeq = SmallVector<Inst, 8>;

InstSeq generateInstSeq(int64_t Val, const MCSubtargetInfo &STI,
                        int64_t *Residual = nullptr);

} // namespace DCMatInt

} // namespace llvm

#endif
