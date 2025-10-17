#include "PassManager.hpp"

#include <llvm/Support/WithColor.h>

namespace glu::optimizer {

PassManager::PassManager(
    DiagnosticManager &diagManager, SourceManager &sourceManager,
    llvm::raw_ostream &output, gil::Module *module,
    llvm::BumpPtrAllocator &gilArena
)
    : _diagManager(diagManager)
    , _sourceManager(sourceManager)
    , _output(output)
    , _printer(&sourceManager, output)
    , _module(module)
    , _gilArena(gilArena)
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

void PassManager::runPasses()
{
#define GIL_PASS(NAME, CLASS)                                 \
    if (!options::isDisabled(NAME)) {                         \
        if (options::hasPrintBeforeEachPasses()               \
            || options::hasPrintBefore(NAME))                 \
            printModule(_module, "GIL before " NAME " pass"); \
        run##CLASS();                                         \
        if (options::hasPrintAfterEachPasses()                \
            || options::hasPrintAfter(NAME))                  \
            printModule(_module, "GIL after " NAME " pass");  \
    }

#include "GILPasses.def"

#undef GIL_PASS
}

} // namespace glu::optimizer
