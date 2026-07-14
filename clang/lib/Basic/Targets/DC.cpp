#include "DC.h"

using namespace clang;
using namespace clang::targets;

DCTargetInfo::DCTargetInfo(const llvm::Triple &Triple,
                           const TargetOptions &Opts)
    : TargetInfo(Triple) {
  if (Triple.isArch32Bit()) {
    LongLongAlign = 32;
    Int128Align = 32;
  } else {
    PointerWidth = PointerAlign = 64;
    LongWidth = LongAlign = 64;
    Int128Align = 64;
  }

  resetDataLayout();
}

void DCTargetInfo::getTargetDefines(const LangOptions &Opts,
                                    MacroBuilder &Builder) const {}

llvm::SmallVector<Builtin::InfosShard> DCTargetInfo::getTargetBuiltins() const {
  return {};
}

TargetInfo::BuiltinVaListKind DCTargetInfo::getBuiltinVaListKind() const {
  return TargetInfo::VoidPtrBuiltinVaList;
}

bool DCTargetInfo::validateAsmConstraint(
    const char *&Name, TargetInfo::ConstraintInfo &info) const {
  return false;
}

std::string_view DCTargetInfo::getClobbers() const { return ""; }

ArrayRef<const char *> DCTargetInfo::getGCCRegNames() const {
  static const char *const GCCRegNames[] = {
      "r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",  "r8",  "r9",
      "r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17", "r18", "r19",
      "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29",
      "r30", "r31", "r32", "r33", "r34", "r35", "r36", "r37", "r38", "r39",
      "r40", "r41", "r42", "r43", "r44", "r45", "r46", "r47", "r48", "r49",
      "r50", "r51", "r52", "r53", "r54", "r55", "r56", "r57", "r58", "r59",
      "r60", "r61", "r62", "r63",
  };

  return llvm::ArrayRef(GCCRegNames);
}

ArrayRef<TargetInfo::GCCRegAlias> DCTargetInfo::getGCCRegAliases() const {
  return {};
}
