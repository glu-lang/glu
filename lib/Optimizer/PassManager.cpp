#include "PassManager.hpp"

#include <llvm/Support/WithColor.h>

namespace glu::optimizer {

PassManager::PassManager(
    SourceManager *sourceManager, llvm::raw_ostream &output
)
    : _printer(sourceManager, output), _output(output)
{
}

void PassManager::printModule(gil::Module *module, llvm::StringRef description)
{
    llvm::WithColor(llvm::outs(), llvm::raw_ostream::CYAN, true)
        << "// " << description << "\n";
    _output << "\n";
    _printer.visit(module);
    _output << "\n";
    llvm::WithColor(llvm::outs(), llvm::raw_ostream::CYAN, true)
        << "// End " << description << "\n\n";
}

void PassManager::runPasses(
    gil::Module *module, llvm::BumpPtrAllocator &arena,
    DiagnosticManager &diagManager
)
{
#define GIL_PASS(NAME, CLASS)                                \
    if (!PassManagerOptions::isDisabled(NAME)) {             \
        if (PassManagerOptions::hasPrintBefore(NAME)) {      \
            printModule(module, "GIL before " NAME " pass"); \
        }                                                    \
        run##CLASS(module, arena, diagManager);              \
        if (PassManagerOptions::hasPrintAfter(NAME)) {       \
            printModule(module, "GIL after " NAME " pass");  \
        }                                                    \
    }

#include "GILPasses.def"

#undef GIL_PASS
}

} // namespace glu::optimizer
