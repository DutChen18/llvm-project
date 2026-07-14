#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_DC_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_DC_H

#include "clang/Basic/TargetInfo.h"

namespace clang {
namespace targets {

class DCTargetInfo : public TargetInfo {
public:
  DCTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts);

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  llvm::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override;

  BuiltinVaListKind getBuiltinVaListKind() const override;

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &info) const override;

  std::string_view getClobbers() const override;

protected:
  ArrayRef<const char *> getGCCRegNames() const override;

  ArrayRef<GCCRegAlias> getGCCRegAliases() const override;
};

using DC32TargetInfo = DCTargetInfo;
using DC64TargetInfo = DCTargetInfo;

} // namespace targets
} // namespace clang

#endif
