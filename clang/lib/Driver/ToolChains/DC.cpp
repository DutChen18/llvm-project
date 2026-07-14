#include "DC.h"
#include "clang/Driver/Compilation.h"

using namespace clang;
using namespace clang::driver;
using namespace clang::driver::toolchains;
using namespace clang::driver::tools;

dc::Assembler::Assembler(const ToolChain &TC)
    : Tool("dc::Assembler", "assembler", TC) {}

bool dc::Assembler::hasIntegratedCPP() const { return false; }

void dc::Assembler::ConstructJob(Compilation &C, const JobAction &JA,
                                 const InputInfo &Output,
                                 const InputInfoList &Inputs,
                                 const llvm::opt::ArgList &TCArgs,
                                 const char *LinkingOutput) const {
  llvm::opt::ArgStringList CmdArgs;

  assert(Output.isFilename());
  CmdArgs.push_back(Output.getFilename());

  assert(Inputs.size() == 1);
  assert(Inputs[0].isFilename());
  CmdArgs.push_back(Inputs[0].getFilename());

  const char *Exec = TCArgs.MakeArgString(getToolChain().GetProgramPath("as"));
  C.addCommand(std::make_unique<Command>(JA, *this, ResponseFileSupport::None(),
                                         Exec, CmdArgs, Inputs, Output));
}

dc::Linker::Linker(const ToolChain &TC) : Tool("dc::Linker", "linker", TC) {}

bool dc::Linker::hasIntegratedCPP() const { return false; }

void dc::Linker::ConstructJob(Compilation &C, const JobAction &JA,
                              const InputInfo &Output,
                              const InputInfoList &Inputs,
                              const llvm::opt::ArgList &TCArgs,
                              const char *LinkingOutput) const {
  llvm::opt::ArgStringList CmdArgs;

  assert(Output.isFilename());
  CmdArgs.push_back(Output.getFilename());

  for (const InputInfo &Input : Inputs) {
    assert(Input.isFilename());
    CmdArgs.push_back(Input.getFilename());
  }

  CmdArgs.push_back(getToolChain().getCompilerRTArgString(TCArgs, "builtins"));

  const char *Exec =
      TCArgs.MakeArgString(getToolChain().GetProgramPath("link"));
  C.addCommand(std::make_unique<Command>(JA, *this, ResponseFileSupport::None(),
                                         Exec, CmdArgs, Inputs, Output));
}

DCToolChain::DCToolChain(const Driver &D, const llvm::Triple &Triple,
                         const llvm::opt::ArgList &Args)
    : ToolChain(D, Triple, Args) {
  // TODO: un-hardcode
  getProgramPaths().push_back("/home/chen/dev/cpu/target/debug");
}

Tool *DCToolChain::buildAssembler() const {
  return new tools::dc::Assembler(*this);
}

Tool *DCToolChain::buildLinker() const { return new tools::dc::Linker(*this); }

bool DCToolChain::isPICDefault() const { return false; }

bool DCToolChain::isPIEDefault(const llvm::opt::ArgList &Args) const {
  return false;
}

bool DCToolChain::isPICDefaultForced() const { return false; }

bool DCToolChain::useIntegratedAs() const { return false; }
