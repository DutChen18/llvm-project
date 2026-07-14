#ifndef LLVM_LIB_TARGET_DC_MCTARGETDESC_DCMCTARGETDESC_H
#define LLVM_LIB_TARGET_DC_MCTARGETDESC_DCMCTARGETDESC_H

#define GET_INSTRINFO_ENUM
#include "DCGenInstrInfo.inc"

#define GET_REGINFO_ENUM
#include "DCGenRegisterInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "DCGenSubtargetInfo.inc"

#endif
