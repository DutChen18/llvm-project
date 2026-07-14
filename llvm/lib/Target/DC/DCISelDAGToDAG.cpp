#include "DCISelDAGToDAG.h"
#include "DC.h"
#include "DCSelectionDAGInfo.h"
#include "DCTargetMachine.h"
#include "MCTargetDesc/DCMCTargetDesc.h"
#include "MCTargetDesc/DCMatInt.h"

using namespace llvm;

#define GET_DAGISEL_BODY DCDAGToDAGISel
#include "DCGenDAGISel.inc"

static SDValue selectImm(SelectionDAG *CurDAG, const SDLoc &DL, MVT VT,
                         int64_t Imm, const DCSubtarget &Subtarget,
                         int64_t *Residual = nullptr) {
  SDValue SrcReg =
      CurDAG->getCopyFromReg(CurDAG->getEntryNode(), DL, DC::R0, VT);

  for (const DCMatInt::Inst &Inst :
       DCMatInt::generateInstSeq(Imm, Subtarget, Residual)) {
    SDValue SDImm = CurDAG->getSignedTargetConstant(Inst.getImm(), DL, VT);
    SDNode *Result;

    switch (Inst.getInstKind()) {
    case DCMatInt::Imm:
      Result = CurDAG->getMachineNode(Inst.getOpcode(), DL, VT, SDImm);
      break;
    case DCMatInt::ImmReg:
      Result = CurDAG->getMachineNode(Inst.getOpcode(), DL, VT, SDImm, SrcReg);
      break;
    case DCMatInt::RegImm:
      Result = CurDAG->getMachineNode(Inst.getOpcode(), DL, VT, SrcReg, SDImm);
      break;
    }

    SrcReg = SDValue(Result, 0);
  }

  return SrcReg;
}

DCDAGToDAGISel::DCDAGToDAGISel(DCTargetMachine &TM, CodeGenOptLevel OptLevel)
    : SelectionDAGISel(TM, OptLevel) {}

bool DCDAGToDAGISel::runOnMachineFunction(MachineFunction &MF) {
  Subtarget = &MF.getSubtarget<DCSubtarget>();
  return SelectionDAGISel::runOnMachineFunction(MF);
}

void DCDAGToDAGISel::Select(SDNode *Node) {
  SDLoc DL(Node);
  MVT VT = Node->getSimpleValueType(0);

  switch (Node->getOpcode()) {
  case ISD::Constant: {
    assert(VT == Subtarget->getRLenVT());

    int64_t Imm = cast<ConstantSDNode>(Node)->getSExtValue();
    ReplaceNode(Node, selectImm(CurDAG, DL, VT, Imm, *Subtarget).getNode());
    return;
  }
  }

  SelectCode(Node);
}

bool DCDAGToDAGISel::SelectAddrRegImm(SDValue Addr, SDValue &Base,
                                      SDValue &Offset) {
  SDLoc DL(Addr);
  MVT VT = Addr.getSimpleValueType();
  int64_t Val = 0;

  assert(VT == Subtarget->getRLenVT());

  if (auto *FIN = dyn_cast<FrameIndexSDNode>(Addr)) {
    Addr = CurDAG->getTargetFrameIndex(FIN->getIndex(), VT);
  } else if (auto *CN = dyn_cast<ConstantSDNode>(Addr)) {
    Addr = selectImm(CurDAG, DL, VT, CN->getSExtValue(), *Subtarget, &Val);
  } else if (CurDAG->isBaseWithConstantOffset(Addr)) {
    Val = cast<ConstantSDNode>(Addr.getOperand(1))->getSExtValue();
    Addr = Addr.getOperand(0);

    if (Val >= -4096 && Val <= 4094) {
      if (auto *FIN = dyn_cast<FrameIndexSDNode>(Addr))
        Addr = CurDAG->getTargetFrameIndex(FIN->getIndex(), VT);

      if (!isInt<12>(Val)) {
        int64_t Adj = Val < 0 ? -2048 : 2047;

        Addr = SDValue(CurDAG->getMachineNode(
                           Subtarget->getADDI(), DL, VT,
                           CurDAG->getSignedTargetConstant(Adj, DL, VT), Addr),
                       0);

        Val -= Adj;
      }
    } else
      Addr =
          SDValue(CurDAG->getMachineNode(
                      Subtarget->getADD(), DL, VT,
                      selectImm(CurDAG, DL, VT, Val, *Subtarget, &Val), Addr),
                  0);
  }

  Base = Addr;
  Offset = CurDAG->getSignedTargetConstant(Val, DL, VT);
  return true;
}

char DCDAGToDAGISelLegacy::ID = 0;

DCDAGToDAGISelLegacy::DCDAGToDAGISelLegacy(DCTargetMachine &TM,
                                           CodeGenOptLevel OptLevel)
    : SelectionDAGISelLegacy(ID,
                             std::make_unique<DCDAGToDAGISel>(TM, OptLevel)) {}

FunctionPass *llvm::createDCISelDag(DCTargetMachine &TM,
                                    CodeGenOptLevel OptLevel) {
  return new DCDAGToDAGISelLegacy(TM, OptLevel);
}

INITIALIZE_PASS(DCDAGToDAGISelLegacy, "dc-isel",
                "DC DAG->DAG Pattern Instruction Selection", false, false)
