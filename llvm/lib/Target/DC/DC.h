#ifndef LLVM_LIB_TARGET_DC_DC_H
#define LLVM_LIB_TARGET_DC_DC_H

#include "llvm/Support/CodeGen.h"

namespace llvm {

class PassRegistry;
class DCTargetMachine;
class FunctionPass;

FunctionPass *createDCISelDag(DCTargetMachine &TM, CodeGenOptLevel OptLevel);

void initializeDCAsmPrinterPass(PassRegistry &);
void initializeDCDAGToDAGISelLegacyPass(PassRegistry &);

} // namespace llvm

#endif
