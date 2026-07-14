#include "DCMCTargetDesc.h"
#include "DCInstPrinter.h"
#include "DCMCAsmInfo.h"
#include "DCTargetStreamer.h"
#include "TargetInfo/DCTargetInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "DCGenInstrInfo.inc"

#define GET_REGINFO_MC_DESC
#include "DCGenRegisterInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "DCGenSubtargetInfo.inc"

using namespace llvm;

static MCAsmInfo *createDCMCAsmInfo(const MCRegisterInfo &MRI, const Triple &TT,
                                    const MCTargetOptions &Options) {
  return new DCMCAsmInfo(Options);
}

static MCInstrInfo *createDCMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitDCMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createDCMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitDCMCRegisterInfo(X, DC::R1);
  return X;
}

static MCInstPrinter *createDCMCInstPrinter(const Triple &T,
                                            unsigned SyntaxVariant,
                                            const MCAsmInfo &MAI,
                                            const MCInstrInfo &MII,
                                            const MCRegisterInfo &MRI) {
  return new DCInstPrinter(MAI, MII, MRI);
}

static MCSubtargetInfo *createDCMCSubtargetInfo(const Triple &TT, StringRef CPU,
                                                StringRef FS) {
  return createDCMCSubtargetInfoImpl(TT, CPU, CPU, FS);
}

static MCTargetStreamer *createDCAsmTargetStreamer(MCStreamer &S,
                                                   formatted_raw_ostream &OS,
                                                   MCInstPrinter *InstPrint) {
  return new DCTargetStreamer(S);
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeDCTargetMC() {
  for (Target *T : {&getTheDC32leTarget(), &getTheDC64leTarget(),
                    &getTheDC32beTarget(), &getTheDC64beTarget()}) {
    TargetRegistry::RegisterMCAsmInfo(*T, createDCMCAsmInfo);
    TargetRegistry::RegisterMCInstrInfo(*T, createDCMCInstrInfo);
    TargetRegistry::RegisterMCRegInfo(*T, createDCMCRegisterInfo);
    TargetRegistry::RegisterMCInstPrinter(*T, createDCMCInstPrinter);
    TargetRegistry::RegisterMCSubtargetInfo(*T, createDCMCSubtargetInfo);
    TargetRegistry::RegisterAsmTargetStreamer(*T, createDCAsmTargetStreamer);
  }
}
