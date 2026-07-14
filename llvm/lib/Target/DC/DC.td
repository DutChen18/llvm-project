include "llvm/Target/Target.td"

include "DCFeatures.td"

include "DCRegisterInfo.td"
include "DCCallingConv.td"
include "DCInstrInfo.td"

include "DCProcessors.td"

def DCInstrInfo : InstrInfo;

defm : RemapAllTargetPseudoPointerOperands<GP>;

def DC : Target {
  let InstructionSet = DCInstrInfo;
  let AllowRegisterRenaming = true;
}
