add_llvm_component_library(LLVMDCInfo
  DCTargetInfo.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  DC
  )
