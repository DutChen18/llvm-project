#include "DCSelectionDAGInfo.h"

#define GET_SDNODE_DESC
#include "DCGenSDNodeInfo.inc"

using namespace llvm;

DCSelectionDAGInfo::DCSelectionDAGInfo()
    : SelectionDAGGenTargetInfo(DCGenSDNodeInfo) {}
