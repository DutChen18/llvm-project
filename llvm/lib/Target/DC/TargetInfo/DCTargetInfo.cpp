#include "DCTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

Target &llvm::getTheDC32leTarget() {
  static Target TheDC32leTarget;
  return TheDC32leTarget;
}

Target &llvm::getTheDC64leTarget() {
  static Target TheDC64leTarget;
  return TheDC64leTarget;
}

Target &llvm::getTheDC32beTarget() {
  static Target TheDC32beTarget;
  return TheDC32beTarget;
}

Target &llvm::getTheDC64beTarget() {
  static Target TheDC64beTarget;
  return TheDC64beTarget;
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeDCTargetInfo() {
  RegisterTarget<Triple::dc32le> X(getTheDC32leTarget(), "dc32le",
                                   "32-bit little endian", "DC");
  RegisterTarget<Triple::dc64le> Y(getTheDC64leTarget(), "dc64le",
                                   "64-bit little endian", "DC");
  RegisterTarget<Triple::dc32be> A(getTheDC32beTarget(), "dc32be",
                                   "32-bit big endian", "DC");
  RegisterTarget<Triple::dc64be> B(getTheDC64beTarget(), "dc64be",
                                   "64-bit big endian", "DC");
}
