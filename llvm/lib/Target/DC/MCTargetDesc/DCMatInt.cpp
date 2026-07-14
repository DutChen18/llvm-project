#include "DCMatInt.h"
#include "DCMCTargetDesc.h"
#include "llvm/MC/MCSubtargetInfo.h"

using namespace llvm;

DCMatInt::Inst::Inst(unsigned Opcode, int64_t Imm) : Opcode(Opcode), Imm(Imm) {}

unsigned DCMatInt::Inst::getOpcode() const { return Opcode; }

int64_t DCMatInt::Inst::getImm() const { return Imm; }

DCMatInt::InstKind DCMatInt::Inst::getInstKind() const {
  switch (Opcode) {
  default:
    llvm_unreachable("unexpected opcode");
  case DC::LUI:
    return DCMatInt::Imm;
  case DC::ADDIW:
  case DC::ADDI:
    return DCMatInt::ImmReg;
  case DC::SLLI:
    return DCMatInt::RegImm;
  }
}

DCMatInt::InstSeq DCMatInt::generateInstSeq(int64_t Val,
                                            const MCSubtargetInfo &STI,
                                            int64_t *Residual) {
  bool IsDC64 = STI.hasFeature(DC::Feature64Bit);
  int64_t Lo12 = SignExtend64<12>(Val);
  int64_t Hi = Val - Lo12;
  InstSeq Res;

  assert(IsDC64 || isInt<32>(Val));

  if (isInt<32>(Val) || isInt<32>(Hi)) {
    int64_t Hi20 = SignExtend64<20>(Hi >> 12);

    if (Hi20 != 0)
      Res.emplace_back(DC::LUI, Hi20);

    if (IsDC64 && !isInt<32>(Hi)) {
      Res.emplace_back(DC::ADDIW, Lo12);
      Lo12 = 0;
    }
  } else {
    int Bits = countr_zero((uint64_t)Hi);
    int64_t HiBits = SignExtend64(Hi >> Bits, 64 - Bits);

    if (Bits > 12 && !isInt<12>(HiBits) && isInt<32>(HiBits << 12)) {
      Bits -= 12;
      HiBits <<= 12;
    }

    Res = generateInstSeq(HiBits, STI);
    Res.emplace_back(DC::SLLI, Bits);
  }

  if (Residual)
    *Residual = Lo12;
  else if (Lo12 != 0)
    Res.emplace_back(IsDC64 ? DC::ADDI : DC::ADDIW, Lo12);

  return Res;
}
