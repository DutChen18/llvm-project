#ifndef LLVM_LIB_TARGET_DC_TARGETINFO_DCTARGETINFO_H
#define LLVM_LIB_TARGET_DC_TARGETINFO_DCTARGETINFO_H

namespace llvm {

class Target;

Target &getTheDC32leTarget();
Target &getTheDC64leTarget();
Target &getTheDC32beTarget();
Target &getTheDC64beTarget();

} // namespace llvm

#endif
