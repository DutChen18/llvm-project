#ifndef LLVM_LIB_TARGET_DC_DCSUBTARGET_H
#define LLVM_LIB_TARGET_DC_DCSUBTARGET_H

#include "DCFrameLowering.h"
#include "DCISelLowering.h"
#include "DCInstrInfo.h"
#include "llvm/CodeGen/SelectionDAGTargetInfo.h"

#define GET_SUBTARGETINFO_HEADER
#include "DCGenSubtargetInfo.inc"

namespace llvm {

class DCRegisterInfo;

class DCSubtarget : public DCGenSubtargetInfo {
#define GET_SUBTARGETINFO_MACRO(ATTRIBUTE, DEFAULT, GETTER)                    \
  bool ATTRIBUTE = DEFAULT;
#include "DCGenSubtargetInfo.inc"

  DCFrameLowering FrameLowering;
  DCInstrInfo InstrInfo;
  DCTargetLowering TLInfo;
  std::unique_ptr<const SelectionDAGTargetInfo> TSInfo;

  DCSubtarget &initializeSubtargetDependencies(const Triple &TT, StringRef CPU,
                                               StringRef TuneCPU, StringRef FS);

public:
  DCSubtarget(const Triple &TT, StringRef CPU, StringRef TuneCPU, StringRef FS,
              const TargetMachine &TM);

  const DCFrameLowering *getFrameLowering() const override;

  const DCInstrInfo *getInstrInfo() const override;

  const DCRegisterInfo *getRegisterInfo() const override;

  const DCTargetLowering *getTargetLowering() const override;

  const SelectionDAGTargetInfo *getSelectionDAGInfo() const override;

  bool enableMachineScheduler() const override;

  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);

  bool is32Bit() const;
  bool is64Bit() const;

  unsigned getADDI() const;
  unsigned getADD() const;

  MVT getRLenVT() const;
};

} // namespace llvm

#endif
