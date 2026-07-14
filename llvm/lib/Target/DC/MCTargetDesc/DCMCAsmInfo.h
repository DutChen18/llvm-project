#ifndef LLVM_LIB_TARGET_DC_MCTARGETDESC_DCMCASMINFO_H
#define LLVM_LIB_TARGET_DC_MCTARGETDESC_DCMCASMINFO_H

#include "llvm/MC/MCAsmInfo.h"

namespace llvm {

class DCMCAsmInfo : public MCAsmInfo {
public:
  explicit DCMCAsmInfo(const MCTargetOptions &Options);

  void printSwitchToSection(const MCSection &Section, uint32_t Subsection,
                            const Triple &T, raw_ostream &OS) const override;
};

} // namespace llvm

#endif
