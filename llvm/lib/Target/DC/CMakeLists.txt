add_llvm_component_group(DC)

set(LLVM_TARGET_DEFINITIONS DC.td)

tablegen(LLVM DCGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM DCGenCallingConv.inc -gen-callingconv)
tablegen(LLVM DCGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM DCGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM DCGenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM DCGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM DCGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM DCGenSDNodeInfo.inc -gen-sd-node-info)

add_public_tablegen_target(DCCommonTableGen)

add_llvm_target(DCCodeGen
  DCAsmPrinter.cpp
  DCFrameLowering.cpp
  DCInstrInfo.cpp
  DCISelDAGToDAG.cpp
  DCISelLowering.cpp
  DCRegisterInfo.cpp
  DCSelectionDAGInfo.cpp
  DCSubtarget.cpp
  DCTargetMachine.cpp

  LINK_COMPONENTS
  AsmPrinter
  CodeGen
  Core
  DCDesc
  DCInfo
  MC
  SelectionDAG
  Support 
  Target
  TargetParser

  ADD_TO_COMPONENT
  DC
  )

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
