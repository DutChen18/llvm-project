#include "DCSubtarget.h"
#include "DCSelectionDAGInfo.h"
#include "MCTargetDesc/DCMCTargetDesc.h"

using namespace llvm;

#define DEBUG_TYPE "dc-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "DCGenSubtargetInfo.inc"

DCSubtarget &DCSubtarget::initializeSubtargetDependencies(const Triple &TT,
                                                          StringRef CPU,
                                                          StringRef TuneCPU,
                                                          StringRef FS) {
  if (CPU.empty() || CPU == "generic")
    CPU = TT.isArch64Bit() ? "generic-dc64" : "generic-dc32";
  if (TuneCPU.empty())
    TuneCPU = CPU;
  if (TuneCPU == "generic")
    CPU = TT.isArch64Bit() ? "generic-dc64" : "generic-dc32";

  ParseSubtargetFeatures(CPU, TuneCPU, FS);

  return *this;
}

DCSubtarget::DCSubtarget(const Triple &TT, StringRef CPU, StringRef TuneCPU,
                         StringRef FS, const TargetMachine &TM)
    : DCGenSubtargetInfo(TT, CPU, TuneCPU, FS),
      FrameLowering(initializeSubtargetDependencies(TT, CPU, TuneCPU, FS)),
      InstrInfo(*this), TLInfo(TM, *this) {
  TSInfo = std::make_unique<DCSelectionDAGInfo>();
}

const DCFrameLowering *DCSubtarget::getFrameLowering() const {
  return &FrameLowering;
}

const DCInstrInfo *DCSubtarget::getInstrInfo() const { return &InstrInfo; }

const DCRegisterInfo *DCSubtarget::getRegisterInfo() const {
  return &InstrInfo.getRegisterInfo();
}

const DCTargetLowering *DCSubtarget::getTargetLowering() const {
  return &TLInfo;
}

const SelectionDAGTargetInfo *DCSubtarget::getSelectionDAGInfo() const {
  return TSInfo.get();
}

bool DCSubtarget::enableMachineScheduler() const { return true; }

bool DCSubtarget::is32Bit() const { return IsDC32; }

bool DCSubtarget::is64Bit() const { return IsDC64; }

unsigned DCSubtarget::getADDI() const { return IsDC64 ? DC::ADDI : DC::ADDIW; }

unsigned DCSubtarget::getADD() const { return IsDC64 ? DC::ADD : DC::ADDW; }

MVT DCSubtarget::getRLenVT() const { return IsDC64 ? MVT::i64 : MVT::i32; }
