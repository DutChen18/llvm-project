#ifndef LLVM_LIB_TARGET_DC_MCTARGETDESC_DCTARGETSTREAMER_H
#define LLVM_LIB_TARGET_DC_MCTARGETDESC_DCTARGETSTREAMER_H

#include "llvm/MC/MCStreamer.h"

namespace llvm {

class DCTargetStreamer : public MCTargetStreamer {
public:
  DCTargetStreamer(MCStreamer &S);

  void emitValue(const MCExpr *Value) override;
};

} // namespace llvm

#endif
