def SDT_CallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def SDT_CallSeqEnd : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def SDT_DCCall : SDTypeProfile<0, -1, [SDTCisVT<0, RLenVT>]>;
def SDT_DCSelectCC : SDTypeProfile<1, 5, [SDTCisSameAs<1, 2>, SDTCisVT<3, OtherVT>, SDTCisSameAs<0, 4>, SDTCisSameAs<4, 5>]>;

def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_CallSeqStart, [SDNPHasChain, SDNPOutGlue]>;
def callseq_end : SDNode<"ISD::CALLSEQ_END", SDT_CallSeqEnd, [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;
def dc_call : SDNode<"DCISD::CALL", SDT_DCCall, [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPVariadic]>;
def dc_ret_glue : SDNode<"DCISD::RET_GLUE", SDTNone, [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def dc_add_lo : SDNode<"DCISD::ADD_LO", SDTIntBinOp>;
def dc_hi : SDNode<"DCISD::HI", SDTIntUnaryOp>;

def to_dc_cond : SDNodeXForm<cond, "return CurDAG->getTargetConstant(N->get(), SDLoc(N), Subtarget->getRLenVT());">;
def to_tframeindex : SDNodeXForm<frameindex, "return CurDAG->getTargetFrameIndex(N->getIndex(), N->getValueType(0));">;

def uimm6 : Operand<RLenVT>, ImmLeaf<RLenVT, "return isUInt<6>(Imm);">;
def simm12 : Operand<RLenVT>, ImmLeaf<RLenVT, "return isInt<12>(Imm);">;
def simm20 : Operand<RLenVT>, ImmLeaf<RLenVT, "return isInt<20>(Imm);">;
def simm12_bb : Operand<OtherVT>;
def simm20_bb : Operand<OtherVT>;
def symbol : Operand<RLenVT>;
def dc_cond: Operand<RLenVT>, PatLeaf<(cond), "", to_dc_cond>;

def AddrRegImm : ComplexPattern<iPTR, 2, "SelectAddrRegImm">;

class DCInstCommon : Instruction {
  let Namespace = "DC";
}

class DCInst<bits<32> mask, list<Predicate> predicates> : DCInstCommon {
  let Predicates = predicates;
}

class Pseudo : DCInstCommon {
  let isPseudo = 1;
}

class ALU_ri<bits<32> mask, string asm, list<Predicate> predicates> : DCInst<mask, predicates> {
  let OutOperandList = (outs GP:$r0);
  let InOperandList = (ins simm12:$im, GP:$r1);
  let AsmString = asm # " $r0, $im, $r1";
}

class ALU_rr<bits<32> mask, string asm, list<Predicate> predicates> : DCInst<mask, predicates> {
  let OutOperandList = (outs GP:$r0);
  let InOperandList = (ins GP:$r2, GP:$r1);
  let AsmString = asm # " $r0, $r2, $r1";
}

class Shift_ri<bits<32> mask, string asm, list<Predicate> predicates> : DCInst<mask, predicates> {
  let OutOperandList = (outs GP:$r0);
  let InOperandList = (ins GP:$r1, uimm6:$im);
  let AsmString = asm # " $r0, $r1, $im";
}

class Shift_rr<bits<32> mask, string asm, list<Predicate> predicates> : DCInst<mask, predicates> {
  let OutOperandList = (outs GP:$r0);
  let InOperandList = (ins GP:$r1, GP:$r2);
  let AsmString = asm # " $r0, $r1, $r2";
}

class BranchCC_rri<bits<32> mask, string asm, list<Predicate> predicates> : DCInst<mask, predicates> {
  let OutOperandList = (outs);
  let InOperandList = (ins GP:$r0, GP:$r1, simm12_bb:$im);
  let AsmString = asm # " $r0, $r1, $im";
  let isTerminator = 1;
  let isBranch = 1;
}

class DCInstLUI<bits<32> mask, string asm, list<Predicate> predicates> : DCInst<mask, predicates> {
  let OutOperandList = (outs GP:$r0);
  let InOperandList = (ins simm20:$im);
  let AsmString = asm # " $r0, $im";
}

class DCInstLUIPC<bits<32> mask, string asm, list<Predicate> predicates> : DCInst<mask, predicates> {
  let OutOperandList = (outs GP:$r0);
  let InOperandList = (ins simm20:$im);
  let AsmString = asm # " $r0, $im";
}

class Load_ri<bits<32> mask, string asm, list<Predicate> predicates> : DCInst<mask, predicates> {
  let OutOperandList = (outs GP:$r0);
  let InOperandList = (ins simm12:$im, GP:$r1);
  let AsmString = asm # " $r0, $im($r1)";
}

class Store_rri<bits<32> mask, string asm, list<Predicate> predicates> : DCInst<mask, predicates> {
  let OutOperandList = (outs);
  let InOperandList = (ins GP:$r0, simm12:$im, GP:$r1);
  let AsmString = asm # " $r0, $im($r1)";
}

class DCInstJALR<bits<32> mask, string asm, list<Predicate> predicates> : DCInst<mask, predicates> {
  let OutOperandList = (outs GP:$r0);
  let InOperandList = (ins GP:$r1, simm12:$im);
  let AsmString = asm # " $r0, $im($r1)";
}

class DCInstJAL<bits<32> mask, string asm, list<Predicate> predicates> : DCInst<mask, predicates> {
  let OutOperandList = (outs GP:$r0);
  let InOperandList = (ins simm20_bb:$im);
  let AsmString = asm # " $r0, $im";
}

def ADDIW : ALU_ri<0x00000000, "addiw", []>;
def ADDI : ALU_ri<0x00040000, "addi", [IsDC64]>;
def SUBIW : ALU_ri<0x00080000, "subiw", []>;
def SUBI : ALU_ri<0x000C0000, "subi", [IsDC64]>;
def ADDW : ALU_rr<0x00000040, "addw", []>;
def ADD : ALU_rr<0x00040040, "add", [IsDC64]>;
def SUBW : ALU_rr<0x00080040, "subw", []>;
def SUB : ALU_rr<0x000C0040, "sub", [IsDC64]>;
def SRAI : Shift_ri<0x04040040, "srai", []>;
def SRA : Shift_rr<0x040C0040, "sra", []>;
def SLLIW : Shift_ri<0x08000040, "slliw", []>;
def SLLW : Shift_rr<0x08080040, "sllw", []>;
def SLLI : Shift_ri<0x08040040, "slli", [IsDC64]>;
def SLL : Shift_rr<0x080C0040, "sll", [IsDC64]>;
def SRLIW : Shift_ri<0x0C000040, "srliw", []>;
def SRLW : Shift_rr<0x0C080040, "srlw", []>;
def SRLI : Shift_ri<0x0C040040, "srli", [IsDC64]>;
def SRL : Shift_rr<0x0C0C0040, "srl", [IsDC64]>;
def ANDI : ALU_ri<0x00000080, "andi", []>;
def ORI : ALU_ri<0x00040080, "ori", []>;
def AND : ALU_rr<0x000000C0, "and", []>;
def OR : ALU_rr<0x000400C0, "or", []>;
def BEQ : BranchCC_rri<0x00000100, "beq", []>;
def BNE : BranchCC_rri<0x00040100, "bne", []>;
def BLT : BranchCC_rri<0x00000140, "blt", []>;
def BGE : BranchCC_rri<0x00040140, "bge", []>;
def BLTU : BranchCC_rri<0x00080140, "bltu", []>;
def BGEU : BranchCC_rri<0x000C0140, "bgeu", []>;
def LUI : DCInstLUI<0x00000180, "lui", []>;
def LUIPC : DCInstLUIPC<0x000001C0, "luipc", []>;
def LB : Load_ri<0x00000200, "lb", []>;
def LH : Load_ri<0x00040200, "lh", []>;
def LW : Load_ri<0x00080200, "lw", []>;
def LD : Load_ri<0x000C0200, "ld", [IsDC64]>;
def SB : Store_rri<0x00000240, "sb", []>;
def SH : Store_rri<0x00040240, "sh", []>;
def SW : Store_rri<0x00080240, "sw", []>;
def SD : Store_rri<0x000C0240, "sd", [IsDC64]>;
def LBU : Load_ri<0x00000280, "lbu", []>;
def LHU : Load_ri<0x00040280, "lhu", []>;
def LWU : Load_ri<0x00080280, "lwu", [IsDC64]>;
def JALR : DCInstJALR<0x000C0280, "jalr", []>;
def JAL : DCInstJAL<0x000002C0, "jal", []>;

def TRAP : DCInst<0xFFFFFFFF, []> {
  let OutOperandList = (outs);
  let InOperandList = (ins);
  let AsmString = ".dw -1";
  let Pattern = [(trap)];
}

def PseudoBR : Pseudo, PseudoInstExpansion<(JAL R0, simm20_bb:$im)> {
  let OutOperandList = (outs);
  let InOperandList = (ins simm20_bb:$im);
  let Pattern = [(br bb:$im)];
  let isTerminator = 1;
  let isBranch = 1;
  let isBarrier = 1;
}

def PseudoSELECT : Pseudo {
  let OutOperandList = (outs GP:$out);
  let InOperandList = (ins GP:$lhs, GP:$rhs, GP:$t, GP:$f, dc_cond:$cc);
  let Pattern = [(set GP:$out, (selectcc GP:$lhs, GP:$rhs, GP:$t, GP:$f, dc_cond:$cc))];
  let usesCustomInserter = 1;
}

def PseudoCALL : Pseudo {
  let OutOperandList = (outs);
  let InOperandList = (ins symbol:$func);
  let Defs = [R1];
  let isCall = 1;
}

def PseudoRET : Pseudo, PseudoInstExpansion<(JALR R0, R1, 0)> {
  let OutOperandList = (outs);
  let InOperandList = (ins);
  let Pattern = [(dc_ret_glue)];
  let isTerminator = 1;
  let isReturn = 1;
  let isBarrier = 1;
}

def ADJCALLSTACKDOWN : Pseudo {
  let OutOperandList = (outs);
  let InOperandList = (ins i32imm:$amt1, i32imm:$amt2);
  let Pattern = [(callseq_start timm:$amt1, timm:$amt2)];
  let Defs = [R2];
  let Uses = [R2];
}

def ADJCALLSTACKUP : Pseudo {
  let OutOperandList = (outs);
  let InOperandList = (ins i32imm:$amt1, i32imm:$amt2);
  let Pattern = [(callseq_end timm:$amt1, timm:$amt2)];
  let Defs = [R2];
  let Uses = [R2];
}

class RevRegSimm12Pat<SDPatternOperator op, DCInst inst, ValueType vt = RLenVT> : Pat<(vt (op (vt GP:$r1), simm12:$im)), (inst simm12:$im, GP:$r1)>;
class Simm12RegPat<SDPatternOperator op, DCInst inst, ValueType vt = RLenVT> : Pat<(vt (op simm12:$im, (vt GP:$r1))), (inst simm12:$im, GP:$r1)>;
class RegRegPat<SDPatternOperator op, DCInst inst, ValueType vt = RLenVT> : Pat<(vt (op (vt GP:$r1), (vt GP:$r2))), (inst GP:$r1, GP:$r2)>;
class RegUimm6Pat<SDPatternOperator op, DCInst inst, ValueType vt = RLenVT> : Pat<(vt (op (vt GP:$r1), uimm6:$im)), (inst GP:$r1, uimm6:$im)>;

def : RevRegSimm12Pat<add, ADDIW, i32>;
def : RevRegSimm12Pat<add, ADDI, i64>;
def : Simm12RegPat<sub, SUBIW, i32>;
def : Simm12RegPat<sub, SUBI, i64>;
def : RegRegPat<add, ADDW, i32>;
def : RegRegPat<add, ADD, i64>;
def : RegRegPat<sub, SUBW, i32>;
def : RegRegPat<sub, SUB, i64>;
def : RegUimm6Pat<sra, SRAI>;
def : RegRegPat<sra, SRA>;
def : RegUimm6Pat<shl, SLLIW, i32>;
def : RegRegPat<shl, SLLW, i32>;
def : RegUimm6Pat<shl, SLLI, i64>;
def : RegRegPat<shl, SLL, i64>;
def : RegUimm6Pat<srl, SRLIW, i32>;
def : RegRegPat<srl, SRLW, i32>;
def : RegUimm6Pat<srl, SRLI, i64>;
def : RegRegPat<srl, SRL, i64>;
def : RevRegSimm12Pat<and, ANDI>;
def : RevRegSimm12Pat<or, ORI>;
def : RegRegPat<and, AND>;
def : RegRegPat<or, OR>;

class BrccPat<CondCode cond, DCInst inst> : Pat<(brcc cond, GP:$r0, GP:$r1, bb:$im), (inst GP:$r0, GP:$r1, simm12_bb:$im)>;

def : BrccPat<SETEQ, BEQ>;
def : BrccPat<SETNE, BNE>;
def : BrccPat<SETLT, BLT>;
def : BrccPat<SETGE, BGE>;
def : BrccPat<SETULT, BLTU>;
def : BrccPat<SETUGE, BGEU>;

class ExtLoadPat<PatFrag op, DCInst inst> : Pat<(op (AddrRegImm GP:$r1, simm12:$im)), (inst simm12:$im, GP:$r1)>;
class TruncStorePat<PatFrag op, DCInst inst> : Pat<(op GP:$r0, (AddrRegImm GP:$r1, simm12:$im)), (inst GP:$r0, simm12:$im, GP:$r1)>;
class LoadPat<DCInst inst, ValueType vt> : Pat<(vt (load (AddrRegImm GP:$r1, simm12:$im))), (inst simm12:$im, GP:$r1)>;
class StorePat<DCInst inst, ValueType vt> : Pat<(store (vt GP:$r0), (AddrRegImm GP:$r1, simm12:$im)), (inst GP:$r0, simm12:$im, GP:$r1)>;

def : ExtLoadPat<sextloadi8, LB>;
def : ExtLoadPat<sextloadi16, LH>;
def : ExtLoadPat<sextloadi32, LW>;
def : ExtLoadPat<extloadi8, LB>;
def : ExtLoadPat<extloadi16, LH>;
def : ExtLoadPat<extloadi32, LW>;
def : ExtLoadPat<zextloadi8, LBU>;
def : ExtLoadPat<zextloadi16, LHU>;
def : ExtLoadPat<zextloadi32, LWU>;

def : TruncStorePat<truncstorei8, SB>;
def : TruncStorePat<truncstorei16, SH>;
def : TruncStorePat<truncstorei32, SW>;

def : LoadPat<LW, i32>;
def : LoadPat<LD, i64>;

def : StorePat<SW, i32>;
def : StorePat<SD, i64>;

def : Pat<(xor GP:$lhs, -1), (SUBI -1, GP:$lhs)>;
def : Pat<(xor GP:$lhs, GP:$rhs), (AND (OR GP:$lhs, GP:$rhs), (SUBI -1, (AND GP:$lhs, GP:$rhs)))>;

def : Pat<(frameindex:$fi), (ADDIW 0, (to_tframeindex $fi))>, Requires<[IsDC32]>;
def : Pat<(frameindex:$fi), (ADDI 0, (to_tframeindex $fi))>, Requires<[IsDC64]>;

def : Pat<(dc_hi tglobaladdr:$hi), (LUI tglobaladdr:$hi)>;
def : Pat<(dc_hi tblockaddress:$hi), (LUI tblockaddress:$hi)>;
def : Pat<(dc_hi tjumptable:$hi), (LUI tjumptable:$hi)>;
def : Pat<(dc_hi tconstpool:$hi), (LUI tconstpool:$hi)>;

def : Pat<(dc_call tglobaladdr:$func), (PseudoCALL tglobaladdr:$func)>;
def : Pat<(dc_call texternalsym:$func), (PseudoCALL texternalsym:$func)>;

def : Pat<(dc_add_lo tglobaladdr:$lo, GP:$hi), (ADDIW tglobaladdr:$lo, GP:$hi)>, Requires<[IsDC32]>;
def : Pat<(dc_add_lo tblockaddress:$lo, GP:$hi), (ADDIW tblockaddress:$lo, GP:$hi)>, Requires<[IsDC32]>;
def : Pat<(dc_add_lo tjumptable:$lo, GP:$hi), (ADDIW tjumptable:$lo, GP:$hi)>, Requires<[IsDC32]>;
def : Pat<(dc_add_lo tconstpool:$lo, GP:$hi), (ADDIW tconstpool:$lo, GP:$hi)>, Requires<[IsDC32]>;

def : Pat<(dc_add_lo tglobaladdr:$lo, GP:$hi), (ADDI tglobaladdr:$lo, GP:$hi)>, Requires<[IsDC64]>;
def : Pat<(dc_add_lo tblockaddress:$lo, GP:$hi), (ADDI tblockaddress:$lo, GP:$hi)>, Requires<[IsDC64]>;
def : Pat<(dc_add_lo tjumptable:$lo, GP:$hi), (ADDI tjumptable:$lo, GP:$hi)>, Requires<[IsDC64]>;
def : Pat<(dc_add_lo tconstpool:$lo, GP:$hi), (ADDI tconstpool:$lo, GP:$hi)>, Requires<[IsDC64]>;
