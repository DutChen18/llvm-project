#include "DCISelLowering.h"
#include "DCSelectionDAGInfo.h"
#include "DCSubtarget.h"
#include "MCTargetDesc/DCMCTargetDesc.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"

using namespace llvm;

#define GET_CALLING_CONV_IMPL
#include "DCGenCallingConv.inc"

static SDValue getTargetNode(GlobalAddressSDNode *N, SelectionDAG &DAG) {
  return DAG.getTargetGlobalAddress(N->getGlobal(), SDLoc(N),
                                    N->getValueType(0), N->getOffset());
}

static SDValue getTargetNode(BlockAddressSDNode *N, SelectionDAG &DAG) {
  return DAG.getTargetBlockAddress(N->getBlockAddress(), N->getValueType(0),
                                   N->getOffset());
}

static SDValue getTargetNode(ConstantPoolSDNode *N, SelectionDAG &DAG) {
  return DAG.getTargetConstantPool(N->getConstVal(), N->getValueType(0),
                                   N->getAlign(), N->getOffset());
}

static SDValue getTargetNode(JumpTableSDNode *N, SelectionDAG &DAG) {
  return DAG.getTargetJumpTable(N->getIndex(), N->getValueType(0));
}

static unsigned getBrCond(ISD::CondCode CC) {
  switch (CC) {
  default:
    llvm_unreachable("unsupported CondCode");
  case ISD::SETEQ:
    return DC::BEQ;
  case ISD::SETNE:
    return DC::BNE;
  case ISD::SETLT:
    return DC::BLT;
  case ISD::SETGE:
    return DC::BGE;
  case ISD::SETULT:
    return DC::BLTU;
  case ISD::SETUGE:
    return DC::BGEU;
  }
}

DCTargetLowering::DCTargetLowering(const TargetMachine &TM,
                                   const DCSubtarget &STI)
    : TargetLowering(TM, STI), Subtarget(STI) {
  MVT RLenVT = STI.getRLenVT();

  addRegisterClass(RLenVT, &DC::GPRegClass);

  computeRegisterProperties(STI.getRegisterInfo());

  setOperationAction({ISD::MUL, ISD::SMUL_LOHI, ISD::UMUL_LOHI, ISD::MULHU,
                      ISD::MULHS, ISD::SDIV, ISD::UDIV, ISD::SREM, ISD::UREM,
                      ISD::SDIVREM, ISD::UDIVREM},
                     RLenVT, Expand);

  setOperationAction({ISD::ROTL, ISD::ROTR}, RLenVT, Expand);
  setOperationAction({ISD::SHL_PARTS, ISD::SRL_PARTS, ISD::SRA_PARTS}, RLenVT,
                     Expand);

  setOperationAction({ISD::BSWAP, ISD::CTTZ, ISD::CTLZ, ISD::CTPOP}, RLenVT,
                     Expand);

  setOperationAction(ISD::SIGN_EXTEND_INREG, {MVT::i1, MVT::i8, MVT::i16},
                     Expand);

  setOperationAction({ISD::SETCC, ISD::SELECT}, RLenVT, Expand);
  setOperationAction({ISD::BRCOND, ISD::BR_JT, ISD::BRIND}, MVT::Other, Expand);

  setOperationAction({ISD::GlobalAddress, ISD::BlockAddress, ISD::ConstantPool,
                      ISD::JumpTable},
                     RLenVT, Custom);

  setOperationAction(ISD::TRAP, MVT::Other, Legal);

  setCondCodeAction({ISD::SETGT, ISD::SETLE, ISD::SETUGT, ISD::SETULE}, RLenVT,
                    Expand);
}

SDValue DCTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  const DCTargetLowering *TLI = Subtarget.getTargetLowering();

  MVT PtrVT = getPointerTy(DAG.getDataLayout());
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  MachineRegisterInfo &MRI = MF.getRegInfo();

  assert(!IsVarArg);

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC);

  for (unsigned i = 0; i < ArgLocs.size(); i++) {
    CCValAssign &VA = ArgLocs[i];
    MVT VT = VA.getValVT();
    assert(VA.getLocVT() == VT);

    if (VA.isRegLoc()) {
      Register VReg = MRI.createVirtualRegister(TLI->getRegClassFor(VT));
      MRI.addLiveIn(VA.getLocReg(), VReg);
      InVals.push_back(DAG.getCopyFromReg(Chain, DL, VReg, VT));
    } else {
      assert(VA.isMemLoc());

      int FI =
          MFI.CreateFixedObject(VT.getStoreSize(), VA.getLocMemOffset(), true);

      InVals.push_back(DAG.getLoad(VT, DL, Chain, DAG.getFrameIndex(FI, PtrVT),
                                   MachinePointerInfo::getFixedStack(MF, FI)));
    }
  }

  return Chain;
}

bool DCTargetLowering::CanLowerReturn(
    CallingConv::ID CallConv, MachineFunction &MF, bool IsVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs, LLVMContext &Context,
    const Type *RetTy) const {
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, RVLocs, Context);
  return CCInfo.CheckReturn(Outs, RetCC);
}

SDValue
DCTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                              bool IsVarArg,
                              const SmallVectorImpl<ISD::OutputArg> &Outs,
                              const SmallVectorImpl<SDValue> &OutVals,
                              const SDLoc &DL, SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();

  assert(!IsVarArg);

  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, RVLocs, *DAG.getContext());
  CCInfo.AnalyzeReturn(Outs, RetCC);

  SDValue Glue;
  SmallVector<SDValue, 4> RetOps(1);

  for (unsigned i = 0; i < RVLocs.size(); i++) {
    CCValAssign &VA = RVLocs[i];
    MVT VT = VA.getValVT();
    assert(VA.getLocVT() == VT);
    assert(VA.isRegLoc());

    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVals[i], Glue);
    Glue = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VT));
  }

  RetOps[0] = Chain;

  if (Glue)
    RetOps.push_back(Glue);

  return DAG.getNode(DCISD::RET_GLUE, DL, MVT::Other, RetOps);
}

SDValue DCTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                    SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc &DL = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  CallingConv::ID CallConv = CLI.CallConv;
  bool IsVarArg = CLI.IsVarArg;

  const DCRegisterInfo *TRI = Subtarget.getRegisterInfo();

  MVT PtrVT = getPointerTy(DAG.getDataLayout());
  MachineFunction &MF = DAG.getMachineFunction();

  assert(!IsVarArg);
  CLI.IsTailCall = false;

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState ArgCCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  ArgCCInfo.AnalyzeCallOperands(Outs, CC);

  unsigned NumBytes = ArgCCInfo.getStackSize();
  Chain = DAG.getCALLSEQ_START(Chain, NumBytes, 0, DL);

  SmallVector<std::pair<Register, SDValue>, 8> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;
  SDValue StackPtr = DAG.getCopyFromReg(Chain, DL, DC::R2, PtrVT);

  for (unsigned i = 0; i < ArgLocs.size(); i++) {
    CCValAssign &VA = ArgLocs[i];
    MVT VT = VA.getValVT();
    assert(VA.getLocVT() == VT);

    if (VA.isRegLoc())
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), OutVals[i]));
    else {
      assert(VA.isMemLoc());

      SDValue Address =
          DAG.getNode(ISD::ADD, DL, PtrVT, StackPtr,
                      DAG.getIntPtrConstant(VA.getLocMemOffset(), DL));

      MemOpChains.push_back(
          DAG.getStore(Chain, DL, OutVals[i], Address,
                       MachinePointerInfo::getStack(MF, VA.getLocMemOffset())));
    }
  }

  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);

  SDValue Glue;

  for (auto &Reg : RegsToPass) {
    Chain = DAG.getCopyToReg(Chain, DL, Reg.first, Reg.second, Glue);
    Glue = Chain.getValue(1);
  }

  if (GlobalAddressSDNode *S = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(S->getGlobal(), DL, PtrVT);
  else if (ExternalSymbolSDNode *S = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(S->getSymbol(), PtrVT);

  SmallVector<SDValue, 8> Ops;

  Ops.push_back(Chain);
  Ops.push_back(Callee);

  for (auto &Reg : RegsToPass)
    Ops.push_back(DAG.getRegister(Reg.first, Reg.second.getValueType()));

  Ops.push_back(DAG.getRegisterMask(TRI->getCallPreservedMask(MF, CallConv)));

  if (Glue)
    Ops.push_back(Glue);

  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);

  Chain = DAG.getNode(DCISD::CALL, DL, NodeTys, Ops);
  Glue = Chain.getValue(1);
  Chain = DAG.getCALLSEQ_END(Chain, NumBytes, 0, Glue, DL);
  Glue = Chain.getValue(1);

  SmallVector<CCValAssign, 16> RVLocs;
  CCState RetCCInfo(CallConv, IsVarArg, MF, RVLocs, *DAG.getContext());
  RetCCInfo.AnalyzeCallResult(Ins, RetCC);

  for (unsigned i = 0; i < RVLocs.size(); i++) {
    CCValAssign &VA = RVLocs[i];
    MVT VT = VA.getValVT();
    assert(VA.getLocVT() == VT);
    assert(VA.isRegLoc());

    SDValue RetValue = DAG.getCopyFromReg(Chain, DL, VA.getLocReg(), VT, Glue);
    Chain = RetValue.getValue(1);
    Glue = RetValue.getValue(2);
    InVals.push_back(RetValue);
  }

  return Chain;
}

SDValue DCTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  default:
    reportFatalInternalError("unimplemented lowerOperation case");
  case ISD::GlobalAddress:
    return lowerGlobalAddress(Op, DAG);
  case ISD::BlockAddress:
    return lowerBlockAddress(Op, DAG);
  case ISD::ConstantPool:
    return lowerConstantPool(Op, DAG);
  case ISD::JumpTable:
    return lowerJumpTable(Op, DAG);
  }
}

MachineBasicBlock *
DCTargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI,
                                              MachineBasicBlock *BB) const {
  switch (MI.getOpcode()) {
  default:
    llvm_unreachable("unexpected instr type to insert");
  case DC::PseudoSELECT:
    return emitSELECT(MI, BB, Subtarget);
  }
}

template <class NodeTy>
SDValue DCTargetLowering::getAddr(NodeTy *N, SelectionDAG &DAG) const {
  SDLoc DL(N);
  MVT VT = getPointerTy(DAG.getDataLayout());
  SDValue Addr = getTargetNode(N, DAG);

  return DAG.getNode(DCISD::ADD_LO, DL, VT, Addr,
                     DAG.getNode(DCISD::HI, DL, VT, Addr));
}

SDValue DCTargetLowering::lowerGlobalAddress(SDValue Op,
                                             SelectionDAG &DAG) const {
  return getAddr(cast<GlobalAddressSDNode>(Op), DAG);
}

SDValue DCTargetLowering::lowerBlockAddress(SDValue Op,
                                            SelectionDAG &DAG) const {
  return getAddr(cast<BlockAddressSDNode>(Op), DAG);
}

SDValue DCTargetLowering::lowerConstantPool(SDValue Op,
                                            SelectionDAG &DAG) const {
  return getAddr(cast<ConstantPoolSDNode>(Op), DAG);
}

SDValue DCTargetLowering::lowerJumpTable(SDValue Op, SelectionDAG &DAG) const {
  return getAddr(cast<JumpTableSDNode>(Op), DAG);
}

MachineBasicBlock *
DCTargetLowering::emitSELECT(MachineInstr &MI, MachineBasicBlock *BB,
                             const DCSubtarget &Subtarget) const {
  MachineFunction *F = BB->getParent();
  MachineFunction::iterator I = std::next(BB->getIterator());
  const BasicBlock *LLVM_BB = BB->getBasicBlock();
  const DCInstrInfo *TII = Subtarget.getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  MachineBasicBlock *IfFalseMBB = F->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *TailMBB = F->CreateMachineBasicBlock(LLVM_BB);

  F->insert(I, IfFalseMBB);
  F->insert(I, TailMBB);

  unsigned CallFrameSize = TII->getCallFrameSizeAt(MI);
  IfFalseMBB->setCallFrameSize(CallFrameSize);
  TailMBB->setCallFrameSize(CallFrameSize);

  TailMBB->splice(TailMBB->end(), BB, std::next(MI.getIterator()), BB->end());
  TailMBB->transferSuccessorsAndUpdatePHIs(BB);

  BB->addSuccessor(IfFalseMBB);
  BB->addSuccessor(TailMBB);

  ISD::CondCode CC = static_cast<ISD::CondCode>(MI.getOperand(5).getImm());

  BuildMI(BB, DL, TII->get(getBrCond(CC)))
      .addReg(MI.getOperand(1).getReg())
      .addReg(MI.getOperand(2).getReg())
      .addMBB(TailMBB);

  IfFalseMBB->addSuccessor(TailMBB);

  BuildMI(*TailMBB, TailMBB->begin(), DL, TII->get(DC::PHI),
          MI.getOperand(0).getReg())
      .addReg(MI.getOperand(3).getReg())
      .addMBB(BB)
      .addReg(MI.getOperand(4).getReg())
      .addMBB(IfFalseMBB);

  MI.eraseFromParent();

  F->getProperties().resetNoPHIs();

  return TailMBB;
}
