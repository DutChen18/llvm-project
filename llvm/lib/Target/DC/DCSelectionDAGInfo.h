#ifndef LLVM_LIB_TARGET_DC_DCSELECTIONDAGINFO_H
#define LLVM_LIB_TARGET_DC_DCSELECTIONDAGINFO_H

#include "llvm/CodeGen/SelectionDAGTargetInfo.h"

#define GET_SDNODE_ENUM
#include "DCGenSDNodeInfo.inc"

namespace llvm {

class DCSelectionDAGInfo : public SelectionDAGGenTargetInfo {
public:
  DCSelectionDAGInfo();
};

} // namespace llvm

#endif
