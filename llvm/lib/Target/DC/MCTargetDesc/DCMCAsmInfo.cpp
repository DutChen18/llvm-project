#include "DCMCAsmInfo.h"
#include "llvm/MC/MCSection.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

DCMCAsmInfo::DCMCAsmInfo(const MCTargetOptions &Options) : MCAsmInfo(Options) {
  GlobalDirective = "\t.global ";
  WeakDirective = "\t.global ";
  HiddenVisibilityAttr = MCSA_Invalid;
  HasDotTypeDotSizeDirective = false;
  HasSingleParameterDotFile = false;
  AsciiDirective = nullptr;
  AscizDirective = nullptr;
  ZeroDirective = "\t.zero ";
  Data8bitsDirective = "\t.db ";
  Data16bitsDirective = "\t.dh ";
  Data32bitsDirective = "\t.dw ";
  Data64bitsDirective = "\t.dd ";
  Align8bitsDirective = "\t.p2align ";
  Align16bitsDirective = nullptr;
  Align32bitsDirective = nullptr;
  Align64bitsDirective = nullptr;
  SupportsAlignmentFillValue = false;
  UsesSetToEquateSymbol = true;
  SetDirective = "\t.set ";
}

void DCMCAsmInfo::printSwitchToSection(const MCSection &Section,
                                       uint32_t Subsection, const Triple &T,
                                       raw_ostream &OS) const {
  OS << "\t.section " << Section.getName() << "\n";
}
