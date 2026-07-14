add_llvm_component_library(LLVMDCDesc
  DCInstPrinter.cpp
  DCMatInt.cpp
  DCMCAsmInfo.cpp
  DCMCTargetDesc.cpp
  DCTargetStreamer.cpp

  LINK_COMPONENTS
  DCInfo
  MC
  Support
  TargetParser

  ADD_TO_COMPONENT
  DC
  )
