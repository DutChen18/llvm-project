#include "DCTargetMachine.h"
#include "DC.h"
#include "TargetInfo/DCTargetInfo.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/Function.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

class DCPassConfig : public TargetPassConfig {
public:
  DCPassConfig(DCTargetMachine &TM, PassManagerBase &PM);

  DCTargetMachine &getDCTargetMachine() const;

  bool addInstSelector() override;
};

static Reloc::Model getEffectiveRelocModel(const Triple &TT,
                                           std::optional<Reloc::Model> RM) {
  return RM.value_or(Reloc::Static);
}

static std::unique_ptr<TargetLoweringObjectFile> createTLOF(const Triple &TT) {
  return std::make_unique<TargetLoweringObjectFileELF>();
}

DCPassConfig::DCPassConfig(DCTargetMachine &TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

DCTargetMachine &DCPassConfig::getDCTargetMachine() const {
  return getTM<DCTargetMachine>();
}

bool DCPassConfig::addInstSelector() {
  addPass(createDCISelDag(getDCTargetMachine(), getOptLevel()));
  return false;
}

DCTargetMachine::DCTargetMachine(const Target &T, const Triple &TT,
                                 StringRef CPU, StringRef FS,
                                 const TargetOptions &Options,
                                 std::optional<Reloc::Model> RM,
                                 std::optional<CodeModel::Model> CM,
                                 CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(
          T, TT.computeDataLayout(Options.MCOptions.getABIName()), TT, CPU, FS,
          Options, getEffectiveRelocModel(TT, RM),
          getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(createTLOF(TT)) {
  initAsmInfo();

  this->Options.EmitAddrsig = false;
}

const DCSubtarget *DCTargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute TuneAttr = F.getFnAttribute("tune-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  std::string CPU =
      CPUAttr.isValid() ? CPUAttr.getValueAsString().str() : TargetCPU;
  std::string TuneCPU =
      TuneAttr.isValid() ? TuneAttr.getValueAsString().str() : CPU;
  std::string FS =
      FSAttr.isValid() ? FSAttr.getValueAsString().str() : TargetFS;

  SmallString<512> Key;

  raw_svector_ostream(Key) << CPU << TuneCPU << FS;

  auto &I = SubtargetMap[Key];

  if (!I) {
    I = std::make_unique<DCSubtarget>(TargetTriple, CPU, TuneCPU, FS, *this);
  }

  return I.get();
}

TargetPassConfig *DCTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new DCPassConfig(*this, PM);
}

TargetLoweringObjectFile *DCTargetMachine::getObjFileLowering() const {
  return TLOF.get();
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeDCTarget() {
  RegisterTargetMachine<DCTargetMachine> X(getTheDC32leTarget());
  RegisterTargetMachine<DCTargetMachine> Y(getTheDC64leTarget());
  RegisterTargetMachine<DCTargetMachine> A(getTheDC32beTarget());
  RegisterTargetMachine<DCTargetMachine> B(getTheDC64beTarget());

  auto *PR = PassRegistry::getPassRegistry();

  initializeDCAsmPrinterPass(*PR);
  initializeDCDAGToDAGISelLegacyPass(*PR);
}
