#include "DC.h"
#include "MCTargetDesc/DCMCTargetDesc.h"
#include "TargetInfo/DCTargetInfo.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

class DCAsmPrinter : public AsmPrinter {
  SmallSet<const MCSymbol *, 32> Symbols;

public:
  static char ID;

  explicit DCAsmPrinter(TargetMachine &TM,
                        std::unique_ptr<MCStreamer> Streamer);

  void emitEndOfAsmFile(Module &M) override;

  void emitInstruction(const MachineInstr *MI) override;

  bool lowerPseudoInstExpansion(const MachineInstr *MI, MCInst &Inst);

  bool lowerOperand(const MachineOperand &MO, MCOperand &MCOp);

  MCOperand createSymbolOperand(const MCSymbol *Symbol);
};

#include "DCGenMCPseudoLowering.inc"

char DCAsmPrinter::ID = 0;

DCAsmPrinter::DCAsmPrinter(TargetMachine &TM,
                           std::unique_ptr<MCStreamer> Streamer)
    : AsmPrinter(TM, std::move(Streamer), ID) {}

void DCAsmPrinter::emitEndOfAsmFile(Module &M) {
  for (const MCSymbol *Symbol : Symbols)
    if (Symbol->isUndefined())
      OutStreamer->emitRawText("\t.extern " + Symbol->getName());
}

void DCAsmPrinter::emitInstruction(const MachineInstr *MI) {
  MCInst Inst;

  if (!lowerPseudoInstExpansion(MI, Inst)) {
    Inst.setOpcode(MI->getOpcode());

    for (const MachineOperand &MO : MI->operands()) {
      MCOperand MCOp;

      if (lowerOperand(MO, MCOp))
        Inst.addOperand(MCOp);
    }
  }

  EmitToStreamer(*OutStreamer, Inst);
}

bool DCAsmPrinter::lowerOperand(const MachineOperand &MO, MCOperand &MCOp) {
  switch (MO.getType()) {
  default:
    report_fatal_error("unknown operand type");
  case MachineOperand::MO_Register:
    MCOp = MCOperand::createReg(MO.getReg());
    return true;
  case MachineOperand::MO_Immediate:
    MCOp = MCOperand::createImm(MO.getImm());
    return true;
  case MachineOperand::MO_MachineBasicBlock:
    MCOp = createSymbolOperand(MO.getMBB()->getSymbol());
    return true;
  case MachineOperand::MO_ConstantPoolIndex:
    MCOp = createSymbolOperand(GetCPISymbol(MO.getIndex()));
    return true;
  case MachineOperand::MO_ExternalSymbol:
    MCOp = createSymbolOperand(GetExternalSymbolSymbol(MO.getSymbolName()));
    return true;
  case MachineOperand::MO_GlobalAddress:
    MCOp = createSymbolOperand(getSymbolPreferLocal(*MO.getGlobal()));
    return true;
  case MachineOperand::MO_RegisterMask:
    return false;
  }
}

MCOperand DCAsmPrinter::createSymbolOperand(const MCSymbol *Symbol) {
  Symbols.insert(Symbol);
  return MCOperand::createExpr(MCSymbolRefExpr::create(Symbol, OutContext));
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeDCAsmPrinter() {
  RegisterAsmPrinter<DCAsmPrinter> X(getTheDC32leTarget());
  RegisterAsmPrinter<DCAsmPrinter> Y(getTheDC64leTarget());
  RegisterAsmPrinter<DCAsmPrinter> A(getTheDC32beTarget());
  RegisterAsmPrinter<DCAsmPrinter> B(getTheDC64beTarget());
}

INITIALIZE_PASS(DCAsmPrinter, "dc-asm-printer", "DC Assembly Printer", false,
                false)
