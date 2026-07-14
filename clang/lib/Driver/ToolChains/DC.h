#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_DC_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_DC_H

#include "clang/Driver/Tool.h"
#include "clang/Driver/ToolChain.h"

namespace clang {
namespace driver {

namespace tools {
namespace dc {

class LLVM_LIBRARY_VISIBILITY Assembler final : public Tool {
public:
  explicit Assembler(const ToolChain &TC);

  bool hasIntegratedCPP() const override;

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  explicit Linker(const ToolChain &TC);

  bool hasIntegratedCPP() const override;

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

} // namespace dc
} // namespace tools

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY DCToolChain final : public ToolChain {
public:
  DCToolChain(const Driver &D, const llvm::Triple &Triple,
              const llvm::opt::ArgList &Args);

  Tool *buildAssembler() const override;
  Tool *buildLinker() const override;

  bool isPICDefault() const override;

  bool isPIEDefault(const llvm::opt::ArgList &Args) const override;

  bool isPICDefaultForced() const override;

  bool useIntegratedAs() const override;
};

} // namespace toolchains

} // namespace driver
} // namespace clang

#endif
